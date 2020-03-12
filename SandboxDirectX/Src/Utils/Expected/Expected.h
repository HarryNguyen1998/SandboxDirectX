#pragma once
// exception handling
#include <exception>
#include <stdexcept>

/* Main idea: The Expected<T> will return an object with type T or an exception when
// the creation of the object is failed */

/* Benefit: + Allows multiple exceptions 
//			+ Switch between error handling and exception throwing
//			+ Teleport across thread boundary, time boundary(save now, throw later)*/
namespace Util {
	template <class T>
	class Expected
	{
		/* Why use a union? => The return value of this class is either result or an exception
		//  Note about union: all members of the union occupy the SAME memory address
		//  In the memory address of an instance of this class, only an instance of T or and exception_ptr
		//  is represent. They are mortal enemy */
		/* Cannot use assignment because assignment assumed the previous value was valid */
	protected:
		union
		{
			T m_result;
			std::exception_ptr m_exceptionPtr;
		};
		bool m_gotResult; // Are we dead yet? Or we have created the result successfully.
		Expected() {} // Internal constructor
	public:
		// Constructor from a constant reference
		Expected(const T& rhs) : m_result(rhs), m_gotResult(true) {}
		// Constructor from a r-value reference (this is a move constructor)
		// A r-value reference is a temporary reference with no memory address like 5, x+1
		// The resources of rhs will be CLAIMED by the classes
		Expected(T&& rhs) : m_result(std::move(rhs)), m_gotResult(true) {}
		// Copy constructor => Copy the resources from another Expected instance
		Expected(const Expected& rhs) : m_gotResult(rhs.m_gotResult) {
			if (m_gotResult) {
				new(&m_result) T(rhs.m_result); // The the program to create a new object at the address spasce
											 // and copy the result from rhs
			}
			else {
				new(&m_exceptionPtr) std::exception_ptr(rhs.m_exceptionPtr);
			}
		}
		// move constructor
		Expected(Expected&& rhs) : m_gotResult(rhs.m_gotResult) {
			if (m_gotResult) {
				new(&m_result) T(std::move(rhs.m_result));
			}
			else {
				new(&m_exceptionPtr) std::exception_ptr(std::move(rhs.m_exceptionPtr));
			}
		}
		// Destructor
		~Expected() {
			using std::exception_ptr;
			// m_exceptionPtr.~std::exception_ptr => this will not parse!!!!
			if (m_gotResult) { m_result.~T(); }
			else { m_exceptionPtr.~exception_ptr(); }
		}

		// Operator overload
		Expected<T>& operator = (const Expected& rhs) {
			m_gotResult = rhs.m_gotResult;
			if (m_gotResult)
				new(&m_result) T(rhs.m_result);
			else
				new(&m_exceptionPtr) std::exception_ptr(rhs.m_exceptionPtr);
			return *this;
		}
		Expected<T>& operator = (const Expected&& rhs) {
			m_gotResult = rhs.m_gotResult;
			if (m_gotResult)
				new(&m_result) T(std::move(rhs.m_result));
			else
				new(&m_exceptionPtr) std::exception_ptr(std::move(rhs.m_exceptionPtr));
			return *this;
		}

		// Swap two Expected instance
		void swap(Expected& rhs) {
			if (m_gotResult) {
				// lhs has a result
				if (rhs.m_gotResult) {
					// rhs also has a result
					using std::swap;
					swap(m_result, rhs.m_result); // only swap result
					// This blasphemy introduces std::swap to the namespace
					// If T has a swap function => use that swap function
					// else pick up std::swap
				}
				else {
					// "else the blasphemy swap"
					// rhs has no result
					auto t = std::move(rhs.m_exceptionPtr); // temporary variable for rhs.exception
					new(&rhs.m_result) T(std::move(m_result)); // give lhs result to rhs (*)
					new(&m_exceptionPtr) std::exception_ptr(t); // give the temporary exception to lhs
					/* Why we need t? Remember result and exception SHARE the memory space
					in (*), the memory space of rhs has been changed to lhs.result => the original
					exception of rhs is LOST in oblivion */
					std::swap(m_gotResult, rhs.m_gotResult); // swap boolean
					// no need to safeguard for a custom swap function
				}
			}
			else {
				if (rhs.m_gotResult) {
					rhs.swap(*this); // why would we need to implement this when we can 
					// reuse "else the blasphemy swap"
				}
				else {
					m_exceptionPtr.swap(rhs.m_exceptionPtr);// swap the exception
					// std::swap(gotResult, rhs.gotResult); // swap the boolean. This is blasphemy
					// The boolean should be the same....
				}
			}
		}
		// Building from exceptions
		static Expected<T> fromException(std::exception_ptr ptr) {
			Expected<T> result; // create an empty template
			result.m_gotResult = false; // No result for you
			new(&result.m_exceptionPtr) std::exception_ptr(std::move(ptr));
			return result;
		}
		static Expected<T> fromException() {
			return fromException(std::current_exception());
		}
		template <class E>
		static Expected<T> fromException(const E& p_exception) {
			if (typeid(p_exception) != typeid(E)) {
				throw std::invalid_argument("slicing dectected");
			} // Slicing is bad.
			return fromException(std::make_exception_ptr(p_exception));
		}
		// Access
		bool isValid() const { return m_gotResult; }
		T& get() {
			if (!m_gotResult)
				std::rethrow_exception(m_exceptionPtr);
			return m_result;
		}
		const T& get() const {
			if (!m_gotResult)
				std::rethrow_exception(m_exceptionPtr);
			return m_result;
		}
		std::exception_ptr getPtr() {
			if (m_gotResult)
				return NULL;
			else
				return m_exceptionPtr;
		}

		// Probing for the exception test
		// This is very slow because of catch-block
		template <class E>
		bool hasException() const {
			try {
				if (!m_gotResult) std::rethrow_exception(m_exceptionPtr);
				// Throw the reception
			}
			catch (const E& object) {
				return true; // If we catch exception of type E => find and dandy
			}
			catch (...) {
				// We catch something else => NOPE!!!
			}
			// We catch something else or no exception at all
			return false;
		}
	};
}