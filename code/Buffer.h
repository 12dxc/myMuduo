#pragma once

#include <vector>
#include <string>
#include <algorithm>

/// A buffer class modeled after org.jboss.netty.buffer.ChannelBuffer
///
/// @code
/// +-------------------+------------------+------------------+
/// | prependable bytes |  readable bytes  |  writable bytes  |
/// |                   |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0      <=      readerIndex   <=   writerIndex    <=     size
/// @endcode
// 网络库底层的缓冲器类型定义
class Buffer
{
public:
    static const size_t kCheapPrepend = 8;
    static const size_t kInitiaSize = 1024;

    explicit Buffer()
        : buffer_(kCheapPrepend + kInitiaSize),
          readerIndex_(kCheapPrepend),
          writerIndex_(kCheapPrepend) {}
    // 返回可使用读缓冲区长度
    size_t readableBytes() const
    {
        return writerIndex_ - readerIndex_;
    }
    // 返回可使用写缓冲区长度
    size_t writableBytes() const
    {
        return buffer_.size() - writerIndex_;
    }
    // 返回头长度
    size_t prependableBytes() const
    {
        return readerIndex_;
    }
    // 返回缓冲区中可读数据的起始地址
    const char *peek() const
    {
        return begin() + readerIndex_;
    }

    // 回收
    void retrieve(size_t len)
    {
        if (len < readableBytes())
        {
            readerIndex_ += len; // 读缓冲区只读了一部分，更新读缓冲区下标即可
        }
        else
        {
            retrieveAll(); // 重置所有
        }
    }
    // 重置读写缓冲区
    void retrieveAll()
    {
        readerIndex_ = writerIndex_ = kCheapPrepend;
    }

    // 把onMessage函数上报的Buffer数据，转成string类型的数据返回
    std::string retrieveAllAsString()
    {
        return retrieveAsString(readableBytes()); // 应用可读取数据的长度
    }

    // 返回读缓冲区的len长数据
    std::string retrieveAsString(size_t len)
    {

        std::string result(peek(), len); // 取出缓冲区可读数据
        retrieve(len);                   // 对缓冲区进行复位操作
        return result;
    }

    // 如果所传参数超过可写缓冲区则扩容
    void ensureWriteableBytes(size_t len)
    {
        if (writableBytes() < len)
        {
            makeSpace(len); // 扩容函数
        }
    }

    // 把[data, data+len]内存上的数据，添加到writable缓冲区当中
    void append(const char *data, size_t len)
    {
        ensureWriteableBytes(len);
        std::copy(data, data + len, beginWrite());
        writerIndex_ += len;
    }

    // 返回可写区的下标
    char *beginWrite()
    {
        return begin() + writerIndex_;
    }
    const char *beginWrite() const
    {
        return begin() + writerIndex_;
    }

    // 从fd上读取数据
    ssize_t readFd(int fd, int *saveErrno);
    // 从fd上写数据
    ssize_t writeFd(int fd, int *saveErrno);

private:
    char *begin()
    {
        return &*buffer_.begin(); // vector底层数组首元素的地址，也就是数组的起始地址
    }

    const char *begin() const
    {
        return &*buffer_.begin();
    }

    // 扩展空间
    void makeSpace(size_t len)
    {
        if (writableBytes() + prependableBytes() < len + kCheapPrepend)
        {
            buffer_.resize(writerIndex_ + len);
        }
        else // 如果读缓冲区还有富余，则向读缓冲区借空间
        {
            size_t readable = readableBytes();
            std::copy(begin() + readerIndex_,
                      begin() + writerIndex_,
                      begin() + kCheapPrepend);
            readerIndex_ = kCheapPrepend;
            writerIndex_ = readerIndex_ + readable;
        }
    }

    std::vector<char> buffer_;
    size_t readerIndex_;
    size_t writerIndex_;
};