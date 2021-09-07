#pragma once

namespace mediasoup
{

template<typename ObserverType>
class CommonObserver {
public:
    void RegisterObserver(ObserverType* obs) {
        m_observer = obs;
    }

    void UnregisterObserver() {
        m_observer = nullptr;
    }
    
    template <typename FuncType>
    void NotifyObserver(FuncType func) {
        if (!m_observer) {
            return;
        }
        func(m_observer);
    }

private:
    ObserverType* m_observer;
};

}