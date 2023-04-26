//
// Created by Deamon on 16.12.22.
//

#include "GBufferVLK.h"
#include "../vk_mem_alloc.h"

GBufferVLK::GBufferVLK(const HGDeviceVLK &device, VkBufferUsageFlags usageFlags, int maxSize, int alignment) : m_device(device) {
    m_usageFlags = usageFlags;
    m_bufferSize = maxSize;
    m_alignment = alignment;

    //Create virtual buffer off this native buffer
    auto allocator = OffsetAllocator::Allocator(m_bufferSize);
    offsetAllocator = std::move(allocator);

    createBuffer(currentBuffer);
}

GBufferVLK::~GBufferVLK() {
    destroyBuffer(currentBuffer);
}

void GBufferVLK::createBuffer(BufferInternal &buffer) {
//Create new buffer for VBO
    VkBufferCreateInfo vbInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    vbInfo.size = m_bufferSize;
    vbInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    vbInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo ibAllocCreateInfo = {};
    ibAllocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
    ibAllocCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

    ERR_GUARD_VULKAN(vmaCreateBuffer(m_device->getVMAAllocator(), &vbInfo, &ibAllocCreateInfo,
                                     &buffer.stagingBuffer,
                                     &buffer.stagingBufferAlloc,
                                     &buffer.stagingBufferAllocInfo));

    // No need to flush stagingBuffer memory because CPU_ONLY memory is always HOST_COHERENT.
    vbInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | m_usageFlags;
    ibAllocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    ibAllocCreateInfo.flags = 0;
    ERR_GUARD_VULKAN(vmaCreateBuffer(m_device->getVMAAllocator(), &vbInfo, &ibAllocCreateInfo,
                                     &buffer.g_hBuffer,
                                     &buffer.g_hBufferAlloc, nullptr));
}

void GBufferVLK::destroyBuffer(BufferInternal &buffer) {
    auto l_device = m_device;
    auto l_buffer = buffer;
    m_device->addDeallocationRecord(
        [l_buffer, l_device]() {
            vmaDestroyBuffer(l_device->getVMAAllocator(), l_buffer.stagingBuffer, l_buffer.stagingBufferAlloc);
            vmaDestroyBuffer(l_device->getVMAAllocator(), l_buffer.g_hBuffer, l_buffer.g_hBufferAlloc);
        }
    );
}

VkResult GBufferVLK::allocateSubBuffer(BufferInternal &buffer, int sizeInBytes, int fakeSize, OffsetAllocator::Allocation &alloc) {
    std::unique_lock<std::mutex> lock(m_mutex, std::defer_lock);

    bool minAddressStrategy = sizeInBytes < fakeSize && fakeSize > 0;

    if (sizeInBytes == 0) {
        alloc = {.offset = 0};
        return VK_SUCCESS;
    }

    uint32_t allocatingSize = sizeInBytes; //Size in bytes
    if (m_alignment > 0) {
        allocatingSize = allocatingSize + m_alignment;
    }
    lock.lock();
    alloc = this->offsetAllocator.allocate(allocatingSize);
    VkResult result = VK_SUCCESS;
    if (alloc.offset == OffsetAllocator::Allocation::NO_SPACE) {
        result = VK_ERROR_OUT_OF_DEVICE_MEMORY;
    }

    if (minAddressStrategy) {
        if (result == VK_SUCCESS && (alloc.offset+fakeSize) > m_bufferSize) {
            this->offsetAllocator.free(alloc);
            result = VK_ERROR_OUT_OF_DEVICE_MEMORY;
        }
    }
    lock.unlock();
    if (result == VK_SUCCESS && m_alignment > 0) {
        int alignDifference = alloc.offset % m_alignment;
        if (alignDifference > 0) {
            alloc.offset = (alloc.offset + ((m_alignment - alignDifference) % m_alignment));
        }        
    }

    return result;
}

void GBufferVLK::deallocateSubBuffer(BufferInternal &buffer, const OffsetAllocator::Allocation &alloc) {
    //Destruction of this virtualBlock happens only in deallocation queue.
    //So it's safe to assume that even if the buffer's handle been changed by the time VirtualFree happens, 
    //the virtualBlock was still not been free'd
    //So there would be no error

    if (alloc.metadata != OffsetAllocator::Allocation::NO_SPACE) {
        auto l_alloc = alloc;
        auto l_weak = weak_from_this();
        m_device->addDeallocationRecord(
            [l_alloc, l_weak]() {
                auto shared = l_weak.lock();
                if (shared == nullptr) return;

                std::unique_lock<std::mutex> lock(shared->m_mutex, std::defer_lock);
                lock.lock();
                shared->offsetAllocator.free(l_alloc);
                lock.unlock();
            });
    }
}

void GBufferVLK::uploadData(void *data, int length) {
    if (length > m_bufferSize) {
        resize(length);
    }

    memcpy(currentBuffer.stagingBufferAllocInfo.pMappedData, data, length);

    uploadFromStaging(0, 0, length);
}

void GBufferVLK::subUploadData(void *data, int offset, int length) {
    if (offset+length > m_bufferSize) {
        resize(offset+length);
    }
    memcpy((uint8_t *)currentBuffer.stagingBufferAllocInfo.pMappedData+offset, data, length);
}

std::shared_ptr<IBuffer> GBufferVLK::mutate(int newSize) {
    resize(newSize);
    return shared_from_this();
}
void GBufferVLK::resize(int newLength) {
    std::unique_lock<std::mutex> lock(m_mutex, std::defer_lock);
    lock.lock();
    auto oldSize = m_bufferSize;
    offsetAllocator.growSize(newLength - oldSize);

    m_bufferSize = newLength;


    BufferInternal newBuffer;
    createBuffer(newBuffer);

    //Reallocate subBuffers and copy their data
    for (std::list<std::weak_ptr<GSubBufferVLK>>::const_iterator it = currentSubBuffers.begin(); it != currentSubBuffers.end(); ++it){
        auto subBuffer = it->lock();
        if (subBuffer != nullptr) {
            subBuffer->m_dataPointer = (uint8_t *)newBuffer.stagingBufferAllocInfo.pMappedData+subBuffer->m_alloc.offset;
        }
    }

    memcpy((uint8_t *)newBuffer.stagingBufferAllocInfo.pMappedData,
           (uint8_t *)currentBuffer.stagingBufferAllocInfo.pMappedData,
           oldSize
    );

    destroyBuffer(currentBuffer);
    currentBuffer = newBuffer;


    for (std::list<std::weak_ptr<GSubBufferVLK>>::const_iterator it = currentSubBuffers.begin(); it != currentSubBuffers.end(); ++it){
        auto subBuffer = it->lock();
        if (subBuffer != nullptr) {
            subBuffer->executeOnChange();
        }
    }

    executeOnChange();
    lock.unlock();
}

static inline uint8_t BitScanMSB(uint32_t mask)
{
#ifdef _MSC_VER
    unsigned long pos;
    if (_BitScanReverse(&pos, mask))
        return static_cast<uint8_t>(pos);
#elif defined __GNUC__ || defined __clang__
    if (mask)
        return 31 - static_cast<uint8_t>(__builtin_clz(mask));
#else
    uint8_t pos = 31;
    uint32_t bit = 1UL << 31;
    do
    {
        if (mask & bit)
            return pos;
        bit >>= 1;
    } while (pos-- > 0);
#endif
    return UINT8_MAX;
}

//fakeSize is used to make sure the subBuffer has enough bytes left till end of main buffer.
//used for allocating data for UBO, when you don't want to suballocate whole size.
//For example if only one bone matrix is used out 220, sizeInBytes will be size of one matrix, while fakeSize is 220 matrices
std::shared_ptr<GBufferVLK::GSubBufferVLK> GBufferVLK::getSubBuffer(int sizeInBytes, int fakeSize) {
    OffsetAllocator::Allocation alloc;

    VkResult res = VK_ERROR_OUT_OF_DEVICE_MEMORY;
    if (sizeInBytes < m_bufferSize) {
        res = allocateSubBuffer(currentBuffer, sizeInBytes, fakeSize, alloc);
    }

    if(res == VK_SUCCESS)
    {
        auto subBuffer = std::make_shared<GSubBufferVLK>(
                                                          shared_from_this(),
                                                          alloc,
                                                          alloc.offset, sizeInBytes, fakeSize,
                                                         (uint8_t *)currentBuffer.stagingBufferAllocInfo.pMappedData+alloc.offset);

        std::unique_lock<std::mutex> lock(m_mutex, std::defer_lock);
        lock.lock();
        currentSubBuffers.push_back(subBuffer);
        subBuffer->m_iterator = std::prev(currentSubBuffers.end());
        lock.unlock();
        return subBuffer;
    }
    else
    {
        resize(std::max<int>(m_bufferSize*2, 1 << (BitScanMSB(m_bufferSize + fakeSize)+1)));

        return getSubBuffer(sizeInBytes, fakeSize);
    }
}

void GBufferVLK::deleteSubBuffer(std::list<std::weak_ptr<GSubBufferVLK>>::const_iterator &it, const OffsetAllocator::Allocation &alloc, int subBuffersize) {
    if (subBuffersize > 0) {
        deallocateSubBuffer(currentBuffer, alloc);
    }
    
    currentSubBuffers.erase(it);
}

void GBufferVLK::uploadFromStaging(int offset, int destOffset, int length) {
    std::lock_guard<std::mutex> lock(dataToBeUploadedMtx);

    VkBufferCopy &vbCopyRegion = dataToBeUploaded.emplace_back();
    vbCopyRegion.srcOffset = offset;
    vbCopyRegion.dstOffset = destOffset;
    vbCopyRegion.size = length;
}

void GBufferVLK::save(int length) {
    uploadFromStaging(0, 0, length);
}

void GBufferVLK::addSubBufferForUpload(const std::weak_ptr<GSubBufferVLK> &buffer) {
    std::lock_guard<std::mutex> lock(dataToBeUploadedMtx);

    subBuffersForUpload.emplace_back(buffer);
}

MutexLockedVector<VkBufferCopy> GBufferVLK::getSubmitRecords() {
    {
        std::lock_guard<std::mutex> lock(dataToBeUploadedMtx);

        struct uploadInterval {size_t start; size_t size;};
        std::vector<uploadInterval> intervals;
        for (const auto &wsubBuffer : subBuffersForUpload) {
            auto subBuffer = wsubBuffer.lock();
            if (subBuffer == nullptr) continue;

            auto &newInterval = intervals.emplace_back();
            newInterval = {.start = subBuffer->getOffset(), .size = subBuffer->getSize()};
        }

        if (lastSubmittedBufferSize != m_bufferSize) {
            if (lastSubmittedBufferSize > 0) {
                auto& newInterval = intervals.emplace_back();
                newInterval = { .start = 0, .size = lastSubmittedBufferSize };
            }

            lastSubmittedBufferSize = m_bufferSize;
        }

        std::sort(intervals.begin(), intervals.end(), [](auto &a, auto &b ) -> bool {
            return
                a.start != b.start
                ? a.start < b.start
                : a.size < b.size;
        });

        if (!intervals.empty()) {
            auto currInterval = intervals[0];
            const static auto calcIntervalEnd = [](decltype(currInterval) interval, int aligment) -> size_t {
                return aligment > 0 ?
                    ((interval.start + interval.size + aligment - 1) / aligment) * aligment:
                    interval.start + interval.size;
            };

            size_t currIntervalEnd = calcIntervalEnd(currInterval, m_alignment);

            for (int i = 1; i < intervals.size(); i++) {
                auto &nextInterval = intervals[i];
                if (currIntervalEnd < nextInterval.start) {
                    VkBufferCopy &vbCopyRegion = dataToBeUploaded.emplace_back();
                    vbCopyRegion.srcOffset = currInterval.start;
                    vbCopyRegion.dstOffset = currInterval.start;
                    vbCopyRegion.size = currInterval.size;

                    currInterval = intervals[i];
                    currIntervalEnd = calcIntervalEnd(currInterval, m_alignment);
                } else {
                    assert(nextInterval.start - currInterval.start >= 0);
                    currInterval.size = std::max<size_t>(currInterval.size, (nextInterval.start - currInterval.start) + nextInterval.size);
                    currIntervalEnd = calcIntervalEnd(currInterval, m_alignment);
                }
            }

            VkBufferCopy &vbCopyRegion = dataToBeUploaded.emplace_back();
            vbCopyRegion.srcOffset = currInterval.start;
            vbCopyRegion.dstOffset = currInterval.start;
            vbCopyRegion.size = currInterval.size;
        }
        int prevSize = subBuffersForUpload.size();
        subBuffersForUpload.clear();
        subBuffersForUpload.reserve(prevSize);
    }


    return MutexLockedVector<VkBufferCopy>(dataToBeUploaded, dataToBeUploadedMtx, true);
}

//----------------------------------------------------------------
//  SubBuffer thing
//----------------------------------------------------------------

GBufferVLK::GSubBufferVLK::GSubBufferVLK(HGBufferVLK parent,
                                         OffsetAllocator::Allocation alloc,
                                         VkDeviceSize offset,
                                         int size, int fakeSize,
                                         uint8_t *dataPointer) : m_parentBuffer(parent) {
    m_alloc = alloc;
    m_offset = offset;
    m_size = size;
    m_fakeSize = fakeSize > 0 ? fakeSize : m_size;
    m_dataPointer = dataPointer;
}

GBufferVLK::GSubBufferVLK::~GSubBufferVLK() {
    m_parentBuffer->deleteSubBuffer(m_iterator, m_alloc, m_size);
}

void GBufferVLK::GSubBufferVLK::uploadData(void *data, int length) {
    if (length > m_size) {
        std::cerr << "invalid dataSize" << std::endl;
    }

    memcpy(m_dataPointer, data, length);

    m_parentBuffer->addSubBufferForUpload(weak_from_this());
//    m_parentBuffer->uploadFromStaging(m_offset, m_offset, length);
}

void GBufferVLK::GSubBufferVLK::subUploadData(void *data, int offset, int length) {
    if (offset + length > m_size) {
        std::cerr << "invalid dataSize" << std::endl;
    }

    memcpy(m_dataPointer + offset, data, length);

    m_parentBuffer->addSubBufferForUpload(weak_from_this());
}

void *GBufferVLK::GSubBufferVLK::getPointer() {
    return m_dataPointer;
}

void GBufferVLK::GSubBufferVLK::save(int length) {
    if (length > m_size) {
        std::cerr << "invalid dataSize" << std::endl;
    }

    m_parentBuffer->addSubBufferForUpload(weak_from_this());
//    m_parentBuffer->uploadFromStaging(m_offset, m_offset, length);
}

size_t GBufferVLK::GSubBufferVLK::getSize() {
    return m_fakeSize;
}
std::shared_ptr<IBuffer> GBufferVLK::GSubBufferVLK::mutate(int newSize) {
    return m_parentBuffer->getSubBuffer(newSize);
};
