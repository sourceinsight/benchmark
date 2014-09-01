#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <iostream>
#include <chrono>
#include <iomanip>
#include <functional>
#include <algorithm>
#include <sstream>
#include <string>
#include <vector>
#include <string.h>
#include <stdlib.h>


namespace benchmark 
{

struct Options 
{
    size_t times_;
    // some other options will be added in the future

    Options()
        : times_(1)
    {}
};

class Context;
class Item 
{
public:
    Item(std::string name, std::function<void(Context*)> func)
        : name_(name),
          func_(func)
    {}

    std::string name() const { return name_; }
    std::function<void(Context*)> func() const { return func_; }

private:
    std::string name_;
    std::function<void(Context*)> func_;
};


class Register 
{
public:
    static Register* Instance() 
    {
        if (instance_ == nullptr) 
        {
            instance_ = new Register();
            ::atexit(destroy);
        }

        return instance_;
    }

    void addItem(Item& item) 
    {
        items_.push_back(item);
    }

    std::vector<Item> getItems() { return items_; }
    
private:
    static void destroy()
    {
        delete instance_;
    }
    
    Register() {}
    Register(const Register& other);
    const Register& operator=(const Register&); 

    static Register* instance_;
    std::vector<Item> items_;
};

Register* Register::instance_;


// global object's consturctor run before the main()
class Wrapper
{
public:
    Wrapper(const std::string& name, std::function<void(Context*)> func) 
    {
        Item item(name, func);
        Register::Instance()->addItem(item);
    }
};


#define FUNC(x, y) x ## y
#define CONCAT(x, y) FUNC(x, y)

#define BENCHMARK(name, func) benchmark::Wrapper CONCAT(wp, __LINE__)((name), (func));

class Status 
{
public:
    Status(size_t times, std::chrono::nanoseconds totalTime)
        : times_(times),
          totalTime_(totalTime)
    {}

    size_t avgTime() const 
    {
        if (times_ <= 0) 
        {
            return 0;
        }

        return totalTime_.count() / times_;
    }

    std::string toString() const 
    {
        std::stringstream str;
        str << std::setw(20) << std::right << times_ 
            << std::setw(20) << std::right << totalTime_.count() 
            << std::setw(20) << std::right << avgTime();
        
        return std::string(str.str());
    }

private:
    size_t times_;
    std::chrono::nanoseconds totalTime_;
};


class Context 
{
public:
    Context(const Item& item, const Options& opts)
        : start_(),
          totalTime_(),
          times_(opts.times_),
          item_(item)
    {}
    
    size_t avgTime() 
    {
        if (times_ <= 0) 
        {
            return 0;
        }
        
        return totalTime_.count() / times_;
    }

    Status run() 
    {
        start_ = std::chrono::high_resolution_clock::now();

        size_t times = times_;
        while (times--)
        {
            item_.func()(this);
        }
        totalTime_ += std::chrono::high_resolution_clock::now() - start_;
        
        return Status(times_, totalTime_);
    }

private:
    std::chrono::high_resolution_clock::time_point start_;
    std::chrono::nanoseconds totalTime_;
    size_t times_;
    Item item_;
};

/*
 * The run function will run the registered benchmarks.
 */
void run(const Options& opts) 
{
    std::cout << std::setw(20) << std::left << "Program-Name" 
        << std::setw(20) << std::right << "Run-Times" 
        << std::setw(20) << std::right << "Total-Time(ns)"
        << std::setw(20) << std::right << "Avg-Time(ns)" 
        << std::endl;

    auto benchmarks = Register::Instance()->getItems();
    for (auto& item : benchmarks) 
    {
        Context c(item, opts);
        auto r = c.run();
        std::cout << std::setw(20) << std::left << item.name() << r.toString() << std::endl;
    }
}

} // namespace benchmark


int main(int argc, char* argv[]) 
{
    benchmark::Options opts;

    if (argc >= 2)
    {
       opts.times_ = atoi(argv[1]);
       std::cout << opts.times_ << std::endl;
    }
    benchmark::run(opts);
    
    return 0;
}

#endif // BENCHMARK_H
