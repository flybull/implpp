#pragma once

#include <map>
#include <list>
#include <string>
#include <memory>
#include <mutex>
#include <cassert>
#include <functional>

// note:
// 1. defined class
// class A final : public utils::impl<A, ...>
// {
//   void on_begin(const std::string& name) override {}
//   void on_success(const std::string& name) override {}
//   void on_error(const std::string& name) override {}
// };
// 2. define unit (in the cpp file), 
// *** Cannot define within a function.
// *** dependent-name from other file (self-name)
// static A::unit<> __unit("self-name", {"dependent-name", "dependent-name"}, 
//     []()->bool {}, /* init method */ 
//     []() {},       /* uninit method */ 
// );

// 3. how to call()
// {
//      ... p;
//      A::instance().init(p);
// }
// 4. done
// 5. assert
// "dependent-name"    mismatch (依赖缺失)
// "duplicate-name"    match    (名字冲突)
// "duplicate init"    check    (重复调用init)
// "regist after init" check    (调用顺序错乱)
// "loop dependency"   check    (存在循环依赖)

namespace utils {

template<typename T, typename ...Args>
class impl {
public:
    typedef impl<T, Args...> IMPL;
    struct unit {
        static void ignore(void) {}
        unit(std::string name,
            std::vector<std::string> depends,
            std::function<bool(Args...)> finit,
            std::function<void(void)> funinit = ignore):
            ready(false), name(name), depends(depends), finit(finit), funinit(funinit)
        {
            IMPL::instance().regist(*this);
        }
        unit(std::string name,
            std::function<bool(Args...)> finit,
            std::function<void(void)> funinit = ignore):
            ready(false), name(name), finit(finit), funinit(funinit)
        {
            IMPL::instance().regist(*this);
        }
        bool                            ready;
        std::string                     name;
        std::vector<std::string>        depends;
        std::function<bool(Args...)>    finit;
        std::function<void(void)>       funinit;
    };
    friend class unit;

    static T& instance() {
        static T self;
        return self;
    }
protected:
    impl() = default;
    virtual ~impl() = default;
private:

    inline void regist(const unit& it) {
        std::lock_guard<std::mutex> guard(mutex);
        assert(binit == false); // regist after init(初始化后存在注册)
        auto m = std::make_shared<unit>(it);
        list_wait.push_back(m);
        auto result = map_unit.insert({it.name, m});
        assert(result.second); // duplicate-name match (名字冲突)
    }

    inline bool check(std::shared_ptr<unit>& m) {
        for (const auto & dep : m->depends) {
            auto it = map_unit.find(dep);
            assert(it != map_unit.end()); // dependent-name mismatch (依赖未注册)
            if (!it->second->ready) {
                return false;
            }
        }
        return true;
    }

public:
    bool init(Args ...args) {
        std::lock_guard<std::mutex> guard(mutex);
        assert(binit == false); // duplicate call init.(重复调用init)
        binit = true;
        while (!list_wait.empty()) {
            auto flag = false;
            auto it = list_wait.begin();
            while (it != list_wait.end()) {
                auto& m = *it;
                if (!check(m)) {
                    ++it;
                    continue;
                }
                on_begin(m->name);
                if (m->finit(args...)) {
                    flag = true;
                    m->ready = true;
                    on_success(m->name);
                    list_run.push_front(*it);
                    it = list_wait.erase(it);
                    continue;
                }
                on_error(m->name);
                binit = false;
                uninit_();
                return false;
            }
            assert(flag); // loop dependency (循环依赖)
        }

        return true;
    }
    void uninit() {
        if (binit) {
            std::lock_guard<std::mutex> guard(mutex);
            binit = false;
            uninit_();
        }
    }
private:
    void uninit_() {
        for (auto rit = list_run.rbegin(); 
            rit != list_run.rend(); ++rit) {
            (*rit)->funinit();
        }
        list_run.clear();
    }
public:
    virtual void on_begin(const std::string& name) {}
    virtual void on_success(const std::string& name) {}
    virtual void on_error(const std::string& name) {}

private:
    bool                                          binit = false;
    std::mutex                                    mutex;
    std::list< std::shared_ptr<unit> >            list_run;
    std::list< std::shared_ptr<unit> >            list_wait;
    std::map<std::string, std::shared_ptr<unit> > map_unit;
};
}