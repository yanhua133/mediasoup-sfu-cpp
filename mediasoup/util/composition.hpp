#ifndef EVENT_EMITTER_COMPOSITION_H
#define EVENT_EMITTER_COMPOSITION_H

namespace event {

  /**
   * \brief Forward declaration of the `private_emitter_t` type.
   */
  template <typename T>
  struct private_emitter_t;

  /**
   * \brief Forward declaration of the `emitter_t` type.
   */
  template <typename T>
  struct emitter_t;

  /**
   * \brief Forward declaration of the `subscription_t` type.
   */
  template <typename T>
  struct subscription_t;

  /**
   * \brief Forward declaration of the `typed_subscription_t` type.
   */
  template <typename T>
  struct typed_subscription_t;

  /**
   * \struct composition_t
   * \brief Placeholder type which defines the type `composition_t`
   * for template specialization purposes.
   */
  template <typename T, typename Arg = void>
  struct composition_t
  {};

  /**
   * \brief Composition of a `private_emitter_t`.
   */
  template <typename Arg>
  struct composition_t<private_emitter_t<Arg>>
  {
    /**
     * \constructor
     */
    composition_t(private_emitter_t<Arg>& emitter)
      : m_emitter(emitter)
    {}

    /**
     * \see private_emitter_t
     */
    template<typename Callable>
    subscription_t<Arg> on(const char* event, Callable&& callback)
    {
      return (m_emitter.on(event, callback));
    }

    /**
     * \see private_emitter_t
     */
    template<typename Callable>
    subscription_t<Arg> once(const char* event, Callable&& callback)
    {
      return (m_emitter.once(event, callback));
    }

    /**
     * \see private_emitter_t
     */
    template <typename Type, typename Callable>
    typed_subscription_t<Arg> on(Callable&& callback)
    {
      return (m_emitter.template on<Type>(callback));
    }

    /**
     * \see private_emitter_t
     */
    template <typename Type, typename Callable>
    typed_subscription_t<Arg> once(Callable&& callback)
    {
      return (m_emitter.template once<Type>(callback));
    }

    /**
     * \see private_emitter_t
     */
    private_emitter_t<Arg>& off(const char* event, const subscription_t<Arg>& subscription)
    {
      return (m_emitter.off(event, subscription));
    }

    /**
     * \see private_emitter_t
     */
    template <typename Type>
    private_emitter_t<Arg>& off(const typed_subscription_t<Arg>& subscription)
    {
      return (m_emitter.template off<Type>(subscription));
    }

    /**
     * \see private_emitter_t
     */
    private_emitter_t<Arg>& emit(const char* event, const Arg& value)
    {
      return (m_emitter.emit(event, value));
    }

    /**
     * \see private_emitter_t
     */
    private_emitter_t<Arg>& emit(const char* event) = delete;

    /**
     * \see private_emitter_t
     */
    template <typename Type>
    private_emitter_t<Arg>& emit(const Type& obj)
    {
      return (m_emitter.template emit<Type>(obj));
    }

    /**
     * \see private_emitter_t
     */
    const std::list<subscription_t<Arg>>& listeners_of(const char* event) const
    {
      return (m_emitter.listeners_of(event));
    }

    /**
     * \see private_emitter_t
     */
    template <typename Type>
    const std::list<typed_subscription_t<Arg>>& listeners_of() const
    {
      return (m_emitter.listeners_of());
    }

  protected:
    private_emitter_t<Arg>& m_emitter;
  };
};

#endif // EVENT_EMITTER_COMPOSITION_H
