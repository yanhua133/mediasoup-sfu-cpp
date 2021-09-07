#ifndef EVENT_EMITTER_EVENT_H
#define EVENT_EMITTER_EVENT_H

#include <string>
#include <type_traits>

/**
 * \namespace event
 */
namespace event
{
  /**
   * \brief Forward declaration of the no_arguments_t type.
   */
  struct no_arguments_t;

  /**
   * \brief Forward declaration of the private_emitter_t type.
   */
  template <typename T>
  struct private_emitter_t;

  /**
   * \class event_t
   * \brief A class wrapping the object associated
   * with an emitted event.
   */
  template <typename T>
  class private_event_t
  {
    const private_emitter_t<T>& m_emitter;

  public:

    /**
     * \constructor
     */
    private_event_t(const private_emitter_t<T>& emitter)
      : m_emitter(emitter)
    {}

    /**
     * \return a constant reference to the emitter
     * associated with this event.
     */
    const private_emitter_t<T>& emitter() const {
      return (m_emitter);
    }
  };

  /**
   * \struct event_t
   */
  template <typename T = no_arguments_t>
  class event_t : public private_event_t<T>
  {
    const std::string m_topic;
    const T           m_argument;

  public:

    event_t(const private_emitter_t<T>& emitter, const std::string& topic, const T& value)
      : private_event_t<T>(emitter),
        m_topic(topic),
        m_argument(value)
    {}

    /**
     * \constructor
     */
    event_t(const private_emitter_t<T>& emitter, const char* topic, const T& value)
      : event_t(emitter, topic != nullptr ? topic : std::string(), value)
    {}

    /**
     * \constructor
     */
    event_t(const private_emitter_t<T>& emitter, const T& value)
      : event_t(emitter, nullptr, value)
    {}

    /**
     * \return a constant reference to the topic
     * object associated with the current event.
     */
    const std::string& topic() const {
      return (m_topic);
    }

    /**
     * \return a constant reference to the value
     * object associated with the current event.
     */
    const T& value() const {
      return (m_argument);
    }
  };

  /**
   * \struct typed_event_t
   */
  template <typename T = no_arguments_t>
  struct typed_event_t : public private_event_t<T>
  {
    const std::type_index m_type_index;
    const void*           m_argument;

  public:

    typed_event_t(const private_emitter_t<T>& emitter, const void* value)
      : private_event_t<T>(emitter),
        m_type_index(typeid(value)),
        m_argument(value)
    {}

    /**
     * \return the event the subscription is associated with
     */
    std::type_index type_index() const {
      return (m_type_index);
    }

    /**
     * \return a constant reference to the value
     * object associated with the current event.
     */
    const void* value() const {
      return (m_argument);
    }
  };
};

#endif // EVENT_EMITTER_EVENT_H
