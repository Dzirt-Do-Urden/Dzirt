#ifndef BUFFERED_CHANNEL_H_
#define BUFFERED_CHANNEL_H_
#include <queue>
#include <mutex>
#include <condition_variable>
template<class T>
class BufferedChannel {
 public:
  explicit BufferedChannel(int size) :capacity(size),closed(false){}

  void Send(T value) {
    std::unique_lock<std::mutex> lock(m);
    if(closed){
      throw std::runtime_error("Channel is closed");
    }
    if(!closed&&capacity==queue_.size()){
      sending.wait(lock,[&](){return queue_.size()<capacity||closed;});
    }
    queue_.push(value);
    receiving.notify_one();
  }

  std::pair<T, bool> Recv() {
    std::unique_lock<std::mutex> lock(m);
    if(closed&&queue_.empty()){
      return std::make_pair(T(),closed);
    }
    if(!closed&&queue_.empty()){
      receiving.wait(lock,[&](){return !queue_.empty()||closed;});
    }
    T elem=queue_.front();
    queue_.pop();
    sending.notify_one();
    return std::make_pair(elem,closed);
  }

  void Close() {
    closed=true;
    sending.notify_all();
    receiving.notify_all();
  }
 private:
  std::queue<T> queue_;
  std::mutex m;
  std::condition_variable sending;
  std::condition_variable receiving;
  int capacity;
  bool closed;
};

#endif // BUFFERED_CHANNEL_H_
 
