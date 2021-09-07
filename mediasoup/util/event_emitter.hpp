#ifndef EVENT_EMITTER_H
#define EVENT_EMITTER_H

#include <string>
#include <list>
#include <unordered_map>
#include <functional>
#include <typeindex>
#include <utility>

#include "event.hpp"
#include "subscription.hpp"

/**
 * \namespace event
 */
namespace event
{
  /**
   * \struct no_arguments_t
   * \brief Placeholder to represent subscriptions that
   * do not take any arguments.
   */
  struct no_arguments_t {};

  /**
   * \class emitter_t
   * \brief An class acting as an observer which is responsible
   * of subscriptions and publications of events.
   */
  template <typename T>
  struct private_emitter_t
  {
    /**
     * \brief The storage meant to associate a string event and
     * a list of callbacks.
     */
    using event_storage_t = std::unordered_map<std::string, std::list<subscription_t<T>>>;

    /**
     * \brief The storage meant to associate a type event and
     * a list of callbacks.
     */
    using type_event_storage_t = std::unordered_map<std::type_index, std::list<typed_subscription_t<T>>>;

    /**
     * \constructor
     */
    private_emitter_t() : m_id(0)
    {}

    /**
     * \brief Registers a callable object associated with the
     * given event name.
     * \param event the event name to associate `callback` with.
     * \param callback a callable object to be invoked when `event`
     * is emitted.
     */
    template<typename Callable>
    subscription_t<T> on(const char* event, Callable&& callback)
    {
      return (event_subscribe(event, REGULAR, callback));
    }

    /**
     * \brief Registers a callable object associated with the
     * given event name once, which means that the given `callback`
     * will be called only once and will not be called subsequently
     * when further `event` are triggered.
     * \param event the event name to associate `callback` with.
     * \param callback a callable object to be invoked when `event`
     * is emitted.
     */
    template<typename Callable>
    subscription_t<T> once(const char* event, Callable&& callback)
    {
      return (event_subscribe(event, ONCE, callback));
    }

    /**
     * \brief Allows to subscribe a callable object to a typed event.
     * Such an event is characterized by the fact that it is not bound
     * to a named event (e.g a string), but a type, which has some
     * differences over named events :
     *  - An event is evaluated at compile-time, since it is associated
     *  with a type.
     *  - It makes it possible to associate semantics with an event.
     *  Take for example the case in which you wish to subscribe to
     *  all events on an emitter instance. Many emitters use custom globbing
     *  characters (such as '*') to denote this wish. However this lacks semantics,
     *  the user of the library must be aware of this particularity, and it cannot be
     *  checked at compile-time.
     *  - One of the biggest disadvantage is that it requires RTTI.
     * \param callback a callable object to be invoked when an event
     * associated with `Type` is emitted.
     */
    template <typename Type, typename Callable>
    typed_subscription_t<T> on(Callable&& callback)
    {
      return (type_subscribe<Type>(callback, REGULAR));
    }

    /**
     * \brief Same as `once(const char* event, Callable&& callback)`,
     * but instead of identifying an event using a string literal,
     * this method allows to subscribe to a type.
     */
    template <typename Type, typename Callable>
    typed_subscription_t<T> once(Callable&& callback)
    {
      return (type_subscribe<Type>(callback, ONCE));
    }

    /**
     * \brief Unsubscribe a callable object from the given `event`.
     * \param event the event you wish to unsubscribe from.
     * \param callback the callable object you wish to unsubscribe from
     * the given `event`. Note that this callable must be the instance
     * which has been registered through the `on` interface.
     */
    private_emitter_t<T>& off(const subscription_t<T>& subscription)
    {
      auto topic = subscription.topic().c_str();

      m_string_events[topic].remove_if([&] (const subscription_t<T>& s) {
        return (s == subscription);
      });
      return (*this);
    }

    /**
     * \brief Unsubscribe a callable object from the typed event identified
     * by `Type`.
     * \param callback the callable object you wish to unsubscribe from the
     * given typed event`.
     */
    template <typename Type>
    private_emitter_t<T>& off(const typed_subscription_t<T>& subscription)
    {
      m_type_events[typeid(Type)].remove_if([&] (const typed_subscription_t<T>& s) {
        return (s == subscription);
      });
      return (*this);
    }

    /**
     * \brief Emits a new event and calls its associated callbacks
     * along with the given value.
     * \param event the event name
     * \param value the value to be forwarded to the callbacks associated
     * with `event`.
     */
    private_emitter_t<T>& emit(const char* event, const T& value)
    {
      auto ev = event_t<T>(*this, event, value);

      m_string_events[event].remove_if([&] (const subscription_t<T>& s) {
	s.function()(ev);
	return (s.type() == ONCE);
      });
      return (emit_(ev));
    }

    /**
     * \brief In its base implementation, this method does not exist,
     * it only exist in the context of emitting a string literal event
     * when there is no arguments to pass to the listeners.
     */
    private_emitter_t<T>& emit(const char* event) = delete;

    /**
     * \brief Broadcasts an event to every event listeners.
     * This does not include typed event listeners, since their
     * event type can change.
     * \param event the event to broadcast.
     */
    private_emitter_t<T>& broadcast(const T& event)
    {
      for (auto& e : m_string_events) {
	emit(e.first, event);
      }
      return (*this);
    }

    /**
     * \brief Emits a new typed event and calls its associated callbacks
     * along with an constant reference to the instance of the given `Type`.
     * This method also notifies `typed_event_t` listeners that a new typed
     * event has been emitted.
     * \param obj the instance of the object of type `Type` associated with
     * the emitted event.
     */
    template <typename Type>
    private_emitter_t<T>& emit(const Type& obj)
    {
      return (emit_<Type>(obj).notify_typed_event(typed_event_t<T>(*this, &obj)));
    }

    /**
     * \brief Returns a list of subscriptions associated with the
     * given `event`.
     * \param event the event of which you would like to obtain
     * the callbacks.
     */
    const std::list<subscription_t<T>>& listeners_of(const char* event) const
    {
      return (m_string_events[event]);
    }

    /**
     * \brief Returns a list of subscriptions associated with the
     * given type.
     * \param event the event of which you would like to obtain
     * the callbacks.
     */
    template <typename Type>
    const std::list<typed_subscription_t<T>>& listeners_of() const
    {
      return (m_type_events[typeid(Type)]);
    }


  private:

    /**
     * \brief Internal method allowing to subscribe a listener to a type
     * and allows to specify the type of the subscription.
     * \param callback the callable object to be invoked when an event
     * associated with the type `Type` has been triggered.
     * \param type the type of the subscription.
     */
    template <typename Type, typename Callable>
    typed_subscription_t<T> type_subscribe(Callable&& callback, type_t type)
    {
      auto interceptor = [callback] (const typed_event_t<T>& event) {
        callback(*static_cast<const Type*>(event.value()));
      };
      auto subscription = typed_subscription_t<T>(*this, typeid(Type), ++m_id, type, interceptor);
      m_type_events[typeid(Type)].push_back(subscription);
      emit_(subscription);
      return (subscription);
    }

    /**
     * \brief Internal method allowing to register a callable object
     * associated with the given event name.
     * \param event the event name to associate `callback` with.
     * \param callback a callable object to be invoked when `event`
     * is emitted.
     */
    template<typename Callable>
    subscription_t<T> event_subscribe(const char* event, type_t type, Callable&& callback)
    {
      auto subscription = subscription_t<T>(*this, event, ++m_id, type, callback);

      m_string_events[event].push_back(subscription);
      emit_(subscription);
      return (subscription);
    }

    /**
     * \brief Emits a new typed event and calls its associated callbacks
     * along with an constant reference to the instance of the given `Type`.
     * This internal method does not notify `typed_event_t` listeners that
     * a typed event has been emitted.
     * \param obj the instance of the object of type `Type` associated with
     * the emitted event.
     */
    template <typename Type>
    private_emitter_t<T>& emit_(const Type& obj)
    {
      auto ev = typed_event_t<T>(*this, &obj);

      m_type_events[typeid(Type)].remove_if([&] (const typed_subscription_t<T>& s) {
        s.function()(ev);
        return (s.type() == ONCE);
      });
      return (*this);
    }

    /**
     * \brief Notifies a typed events to custom listeners.
     * \param e the event to forward to custom listeners
     */
    private_emitter_t<T>& notify_typed_event(const typed_event_t<T>& e)
    {
      m_type_events[typeid(e)].remove_if([&] (const typed_subscription_t<T>& s) {
        s.function()(e);
        return (s.type() == ONCE);
      });
      return (*this);
    }


  protected:

    event_storage_t      m_string_events;
    type_event_storage_t m_type_events;
    identifier_t         m_id;
  };

  /**
   * \struct emitter_t
   * \brief The publicly exposed `emitter_t` type which is the main interface
   * used by the client to subscribe to and publish events.
   */
  template <typename T = no_arguments_t>
  struct emitter_t : public private_emitter_t<T>
  {};

  /**
   * \struct emitter_t<no_arguments_t>
   * \brief An `emitter_t` specialization which is the main interface
   * used by the client to subscribe to and publish to events which do not
   * require an argument.
   */
  template <>
  struct emitter_t<no_arguments_t> : public private_emitter_t<no_arguments_t>
  {
    /**
     * \brief Inheriting from all `emit` overrides from the
     * `private_emitter_t` type.
     */
    using private_emitter_t<no_arguments_t>::emit;

    /**
     * \brief Emits a new event and calls its associated callbacks
     * along with the given value.
     * \param event the event name
     * \param value the value to be forwarded to the callbacks associated
     * with `event`.
     */
    emitter_t<no_arguments_t>& emit(const char* event)
    {
      m_string_events[event].remove_if([&] (const subscription_t<no_arguments_t>& s) {
        s.function()(event_t<no_arguments_t>(*this, event, no_arguments_t()));
        return (s.type() == ONCE);
      });
      private_emitter_t<no_arguments_t>::emit<const char*>(event);
      return (*this);
    }
  };
};

#endif // EVENT_EMITTER_H
