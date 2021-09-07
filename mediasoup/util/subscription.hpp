#ifndef EVENT_EMITTER_SUBSCRIPTION_H
#define EVENT_EMITTER_SUBSCRIPTION_H

#include <cstdint>
#include <functional>
#include <type_traits>
#include "event.hpp"
#include "composition.hpp"

/**
 * \namespace event
 */
namespace event
{
  /**
   * \brief Definition of the opaque identifier type associated
   * with a subscription.
   */
  using identifier_t = uint64_t;

  /**
   * \brief Forward declaration of the `private_emitter_t` type.
   */
  template <typename T>
  struct private_emitter_t;

  /**
   * \enum type_t
   * \brief Describes the different types of subscriptions.
   */
  enum type_t {
    REGULAR,
    ONCE
  };

  /**
   * \struct private_subscription_t
   * \brief A private implementation of a subscription from
   * which publicly exposed subscriptions inherits from.
   */
  template <typename T, typename EventType>
  struct private_subscription_t : public composition_t<private_emitter_t<T>>
  {
    /**
     * \brief The function instance associated with the listener's
     * callback.
     */
    using function_t = std::function<void(const EventType&)>;

    /**
     * \constructor
     */
    private_subscription_t(private_emitter_t<T>& emitter, identifier_t identifier, type_t type, const function_t& callable)
      : composition_t<private_emitter_t<T>>(emitter),
        m_function(callable),
        m_identifier(identifier),
        m_type(type)
    {}

    /**
     * \copy-constructor
     */
    private_subscription_t(const private_subscription_t<T, EventType>& s)
      : composition_t<private_emitter_t<T>>(s.m_emitter),
        m_function(s.m_function),
        m_identifier(s.m_identifier),
        m_type(s.m_type)
    {}

    /**
     * \assignment-operator.
     */
    private_subscription_t<T, EventType>& operator=(private_subscription_t<T, EventType> other) {
      std::swap(this->m_emitter, other.m_emitter);
      std::swap(m_function, other.m_function);
      std::swap(m_identifier, other.m_identifier);
      std::swap(m_type, other.m_type);
      return (*this);
    }

    /**
     * \return the type of the subscription.
     */
    type_t type() const {
      return (m_type);
    }

    /**
     * \return a constant reference to the function instance associated
     * with the listener's callback.
     */
    const function_t& function() const {
      return (m_function);
    }

    /**
     * \return an opaque object representing the subscription identifier.
     */
    identifier_t identifier() const {
      return (m_identifier);
    }

    /**
     * \brief Equality operator definition
     */
    bool operator==(const private_subscription_t<T, EventType>& subscription) const {
      return (&(this->m_emitter) == &(subscription.m_emitter)
	      && this->m_identifier == subscription.m_identifier);
    }

    /**
     * \brief Inequality operator definition.
     */
    bool operator!=(const private_subscription_t<T, EventType>& subscription) const {
      return (!operator==(subscription));
    }

  private:

    function_t       m_function;
    identifier_t     m_identifier;
    type_t           m_type;
  };

  /**
   * \struct subscription_t
   * \brief The publicly exposed `subscription_t` type which internally
   * identifies a subscription on the emitter.
   */
  template <typename T>
  struct subscription_t : public private_subscription_t<T, event_t<T>>
  {
    /**
     * \constructor
     */
    subscription_t(private_emitter_t<T>& emitter, const std::string& topic, identifier_t identifier, type_t type, const typename private_subscription_t<T, event_t<T>>::function_t& callable)
      : private_subscription_t<T, event_t<T>>(emitter, identifier, type, callable),
        m_topic(topic)
    {}

    /**
     * \return the event the subscription is associated with
     */
    const std::string& topic() const {
      return (m_topic);
    }

  private:
    const std::string m_topic;
  };

  /**
   * \struct typed_subscription_t
   * \brief The publicly exposed `typed_subscription_t` type which internally
   * identifies a typed subscription on the emitter.
   */
  template <typename T>
  struct typed_subscription_t : public private_subscription_t<T, typed_event_t<T>>
  {
    using function_t = std::function<void(const typed_event_t<T>&)>;

    /**
     * \constructor
     */
    typed_subscription_t(private_emitter_t<T>& emitter, std::type_index type_index, identifier_t identifier, type_t type, const function_t& callable)
      : private_subscription_t<T, typed_event_t<T>>(emitter, identifier, type, callable),
        m_type_index(type_index)
    {}

    /**
     * \return the type index of the associated type.
     */
    std::type_index type_index() const {
      return (m_type_index);
    }

  private:
    std::type_index m_type_index;
  };
};

#endif // EVENT_EMITTER_SUBSCRIPTION_H
