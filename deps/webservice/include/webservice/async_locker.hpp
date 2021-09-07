//-----------------------------------------------------------------------------
// Copyright (c) 2018 Benjamin Buch
//
// https://github.com/bebuch/webservice
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _webservice__async_lock__hpp_INCLUDED_
#define _webservice__async_lock__hpp_INCLUDED_

#include <atomic>
#include <utility>
#include <functional>

#include <iostream>
#include <iomanip>
#include <mutex>


namespace webservice{


	/// \brief Count async operations and calls a user defined function after
	///        all async operations have returned
	class async_locker{
	public:
		/// \brief Increase a counter on construction and decrese on destruction
		class lock{
		public:
			/// \brief Initialize without lock
			///
			/// Thread safe: Yes.
			lock()
				: locker_(nullptr) {}


			/// \brief Increase the counter
			///
			/// \throw std::runtime_error If no other async operation is still
			///                           running
			///
			/// Thread safe: Yes.
			lock(async_locker& locker)
				: locker_(&locker)
			{
				// lock_count_ must be increast by 1 before first usage. After
				// that it must be decreased by 1. This makes sure that if
				// lock_count_ is 0 the on_last_async() was already fired and
				// no new calles are accepted
				if((*locker_).lock_count_ == 0){
					throw std::runtime_error("async call after shutdown");
				}

				// If the call was valid increas the counter
				++(*locker_).lock_count_;
			}

			lock(lock const&) = delete;

			/// \brief Move ownership of the lock
			///
			/// Thread safe: Yes.
			lock(lock&& other)noexcept
				: locker_(other.locker_.exchange(
					nullptr, std::memory_order_relaxed)) {}

			/// \brief Move ownership of the lock
			///
			/// Thread safe: Yes.
			lock& operator=(lock&& other)noexcept{
				locker_ = other.locker_.exchange(
					nullptr, std::memory_order_relaxed);
				return *this;
			}

			/// \brief Call unlock()
			///
			/// Thread safe: Yes.
			~lock(){
				unlock();
			}

			lock& operator=(lock const&) = delete;

			/// \brief Decrese counter, call callback if count becomes 0
			///
			/// Thread safe: Yes.
			void unlock()noexcept{
				auto locker =
					locker_.exchange(nullptr, std::memory_order_relaxed);
				if(locker && --locker->lock_count_ == 0){
					if(locker->on_last_async_callback_){
						locker->on_last_async_callback_();
					}
				}
			}

			/// \brief true is unlock was not called, false otherwise
			///
			/// Thread safe: Yes.
			bool is_locked()const noexcept{
				return locker_ != nullptr;
			}


		private:
			/// \brief Pointer to locked object
			std::atomic< async_locker* > locker_;
		};


		/// \brief Construct without callback
		async_locker()noexcept{}

		/// \brief Construct with a callback function that is called when the
		///        last async operation returns
		template < typename Fn >
		async_locker(Fn&& fn)noexcept
			: on_last_async_callback_(static_cast< Fn&& >(fn))
		{
			static_assert(std::is_nothrow_copy_constructible< Fn >::value,
				"fn must be nothrow copy constructible");
			static_assert(noexcept(std::declval< Fn >()()),
				"fn must be noexcept callable");
		}


		/// \brief Set a callback function that is called when the
		///        last async operation returns
		///
		/// Thread safe: No. This function must not be called in parallel and
		/// not while last async operation may return.
		template < typename Fn >
		void set_callback(Fn&& fn)noexcept{
			static_assert(std::is_nothrow_copy_constructible< Fn >::value,
				"fn must be nothrow copy constructible");
			static_assert(noexcept(std::declval< Fn >()()),
				"fn must be noexcept callable");

			on_last_async_callback_ = static_cast< Fn&& >(fn);
		}


		/// \brief Generate the first lock object
		///
		/// \throw std::logic_error If first_lock() was called more than one
		///                         time
		/// \throw std::runtime_error If no other async operation is still
		///                           running
		///
		/// Thread safe: Yes.
		lock make_first_lock(){
			// set lock_count_ to 1 if it was 0 only
			std::size_t expected = 0;
			if(!lock_count_.compare_exchange_strong(expected, 1,
				std::memory_order_relaxed)
			){
				throw std::logic_error(
					"async_locker::first_lock() called after first lock.");
			}

			lock result(*this);

			// undo the first increase from compare_exchange_strong
			--lock_count_;

			return result;
		}

		/// \brief Generate a lock object
		///
		/// \throw std::runtime_error If first_lock() has not been called
		///                           before or no other async operation is
		///                           still running
		///
		/// Thread safe: Yes.
		lock make_lock(){
			return lock(*this);
		}

		/// \brief Current count of running async operations
		///
		/// Thread safe: Yes.
		std::size_t count()const noexcept{
			return lock_count_;
		}


	private:
		/// \brief Called when the last async operation returned
		std::function< void() > on_last_async_callback_;

		/// \brief Counter of async operations
		///
		/// Inreased on every lock()/first_lock() construction, decreased on
		/// every destruction.
		std::atomic< std::size_t > lock_count_{0};
	};


}


#endif
