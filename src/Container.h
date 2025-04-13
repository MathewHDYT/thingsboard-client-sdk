#ifndef Container_h
#define Container_h

// Local include.
#include "Helper.h"

// Library includes.
#include <assert.h>


/// @brief Custom std::array or std::vector implementation that contains a partial vector-like interface implementation.
/// Automatically changes internal implementation to use the heap or stack depending on if THINGSBOARD_ENABLE_DYNAMIC is set or not
/// @note Allows to use the exact same method calls independent on if the custom Container implementation or std::vector is used.
/// Support for the vector-like interface is achieved through a simple index that keeps count of the elements that have been instantiated with actual values by the push or insert method.
/// Iterator based support is achieved through returning a pointer, which can be automatically used the same as an iterator implementation tagged as std::random_iterator_tag.
/// This allows to use the most efficient implementation of standard algorithms, while keeping the implementation as simple as possible.
/// 
/// The elements are stored contiguously, which means that elements can be accessed not only through iterators, but also using offsets to regular pointers to elements.
/// This means that a pointer to an element of a vector may be passed to any function that expects a pointer to an element of an array
/// @tparam T The type of the elements in the underlying data container. Must be both CopyAssignable (have a copy assignment operator, for the push_back operation)
/// as well as Default-Constructible (have a default constructor, for the construction of the intial state of the underlying data container)
#if THINGSBOARD_ENABLE_DYNAMIC
template <typename T>
#else 
/// @tparam Capacity Amount of elements that can ever be saved into the underlying data container, allows to wrap a simple c-array and allocate it on the stack.
/// Attempting to allocate more elements is not possible, because the size is fixed at compile-time
template <typename T, size_t Capacity>
#endif // THINGSBOARD_ENABLE_DYNAMIC
class Container {
  public:
    using value_type = T;
    using size_type = size_t;
    using reference = T &;
    using const_reference = T const &;
    using pointer = T *;
    using const_pointer = T const *;
    using iterator = pointer;
    using const_iterator = const_pointer;

#if THINGSBOARD_ENABLE_DYNAMIC
    /// @brief Constructor
    Container() = default;
#else
    /// @brief Default constructor, simply initalizes the underlying c-style array with the necessary capacity.
    /// That capacity always has to be bigger than 0, because initalizing a 0 length c-style array can cause certain compilers to produce errors
    Container()
      : m_elements()
      , m_size(0U)
    {
        static_assert(Capacity > 0);
    }
#endif // THINGSBOARD_ENABLE_DYNAMIC

    /// @brief Creates the constructor with the given amount of elements, either x copies of the passed value or x default constructed instances of the underlying type used.
    /// Simply forwards the arguments to the insert method, meaning if the initally allocated Capacity is not big enough to hold all elements,
    /// then this method will assert and stop the application.
    /// @param count Amount of elements that we want to create and copy the value into
    /// @param value Value that should be copied into the created elements, default = T{}
    Container(size_type count, const_reference value = T{})
      : Container()
    {
        for (; count > 0; --count) {
            push_back(value);
        }
    }

    /// @brief Copies all elements from the given start to exclusively the given end iterator into the underlying data container.
    /// Simply forwards the arguments to the insert method, meaning if the initally allocated Capacity is not big enough to hold all elements,
    /// then this method will assert and stop the application.
    /// @tparam InputIterator Class that allows for forward incrementable access to data
    /// of the given data container, allows for using / passing either std::vector or std::array.
    /// See https://en.cppreference.com/w/cpp/iterator/input_iterator for more information on the requirements of the iterator
    /// @param first Iterator pointing to the first element we want to copy into our underlying data container
    /// @param last Iterator pointing to one past the end of the elements we want to copy into our underlying data container
    template<typename InputIterator>
    Container(InputIterator const & first, InputIterator const & last)
      : Container()
    {
        assign(first, last);
    }

    /// @brief Accesses the begin and end iterator of the given data container and forwards the call to the iterator based constructor.
    /// @tparam Container Class that contains the actual data we want to copy into our internal data container,
    /// requires access to a begin() and end() method, that point to the first element and one past the last element we want to copy respectively.
    /// Both methods need to return an InputIterator, allows for using / passing either std::vector or std::array.
    /// See https://en.cppreference.com/w/cpp/iterator/input_iterator for more information on the requirements of the iterator
    /// @param container Data container with begin() and end() method that we want to copy fully into our underlying data container
    template<typename Container>
    Container(Container const & container)
      : Container(container.begin(), container.end())
    {
        // Nothing to do.
    }

    /// @copydoc Container::Container(InputIterator const &, InputIterator const &)
    template<typename InputIterator>
    void assign(InputIterator const & first, InputIterator const & last) {
        insert(end(), first, last);
    }

    /// @copydoc Container::Container(Container const &)
    template<typename Container>
    void assign(Container const & container) {
        assign(container.begin(), container.end());
    }

    /// @brief Returns whether there are any elements in the underlying data container
    /// @return Whether the underlying data container is empty or not
    bool empty() const {
        return m_size == 0U;
    }

    /// @brief Gets the current amount of elements in the underlying data container
    /// @return The amount of items currently in the underlying data container
    size_type size() const {
        return m_size;
    }

    /// @brief Gets the maximum amount of elements that can be stored in the underlying data container
    /// @return The maximum amount of items that can be stored in the underlying data container
#if THINGSBOARD_ENABLE_DYNAMIC
    size_type capacity() const {
        return m_capacity;
#else
    size_type constexpr capacity() const {
        return Capacity;
#endif // THINGSBOARD_ENABLE_DYNAMIC
    }

    /// @brief Returns an iterator to the first element of the underlying data container.
    /// If the array is empty, the returned iterator will be equal to end()
    /// @return Iterator pointing to the first element of the underlying data container
    pointer begin() {
        return m_elements;
    }

    /// @copydoc Container::begin
    const_pointer begin() const {
        return m_elements;
    }

    /// @copydoc Container::begin
    const_pointer cbegin() const {
        return begin();
    }

    /// @brief Returns a reference to the first element of the array.
    /// If the array is empty this method will assert and stop the application, because there is no valid element to return
    /// @return Reference to the first element of the array
    reference front() {
        assert(m_size != 0U);
        return m_elements[0U];
    }

    /// @copydoc Container::front
    const_reference front() const {
        assert(m_size != 0U);
        return m_elements[0U];
    }

    /// @brief Returns a reference to the last element of the array.
    /// If the array is empty this method will assert and stop the application, because there is no valid element to return
    /// @return Reference to the last element of the array
    reference back() {
        assert(m_size != 0U);
        return m_elements[m_size - 1U];
    }

    /// @copydoc Container::back
    const_reference back() const {
        assert(m_size != 0U);
        return m_elements[m_size - 1U];
    }

    /// @brief Returns a iterator to one-past-the-last element of the underlying data container
    /// @return Iterator pointing to one-past-the-last element of the underlying data container
    pointer end() {
        return m_elements + m_size;
    }

    /// @copydoc Container::end
    const_pointer end() const {
        return m_elements + m_size;
    }

    /// @copydoc Container::end
    const_pointer cend() const {
        return end();
    }

    /// @brief Appends the given element at the end of the underlying data container
    /// @note If the interal data structure is full already then this method will assert and stop the application.
    /// Because if we do not we could cause an out of bounds write, which could possibly overwrite other memory.
    /// Causing hard to debug issues, therefore this behaviour is not allowed in the first place
    /// @param element Element that should be inserted at the end
    void push_back(const_reference element) {
#if THINGSBOARD_ENABLE_DYNAMIC
        increase_capacity();
#else
        assert(m_size < Capacity);
#endif // THINGSBOARD_ENABLE_DYNAMIC
        m_elements[m_size++] = element;
    }

    /// @brief Copies all elements from the given start to exclusively the given end iterator into the underlying data container.
    /// The copying is started from the position before the given iterator (position - 1)
    /// @note If the initally allocated Capacity is not big enough to hold all elements, then this method will assert and stop the application.
    /// It will also assert if the position before the iterator (position - 1) is outside the range of this container,
    /// meaning if we are using an iterator to another container or if we passed an invalid iterator that would cause invalid memory access if dereferenced
    /// @tparam InputIterator Class that allows for forward incrementable access to data
    /// of the given data container, allows for using / passing either std::vector or std::array.
    /// See https://en.cppreference.com/w/cpp/iterator/input_iterator for more information on the requirements of the iterator
    /// @param position Iterator before which the content will be copied too. May be the end() iterator
    /// @param first Iterator pointing to the first element we want to copy into our underlying data container
    /// @param last Iterator pointing to one past the end of the elements we want to copy into our underlying data container
    template<typename InputIterator>
    void insert(const_pointer position, InputIterator const & first, InputIterator const & last) {
        assert(begin() >= --position < end());
#if !THINGSBOARD_ENABLE_DYNAMIC
        assert((m_size + Helper::distance(first, last)) < Capacity);
#endif // !THINGSBOARD_ENABLE_DYNAMIC
        for (auto it = first; it != last; ++it, ++position) {
#if THINGSBOARD_ENABLE_DYNAMIC
            increase_capacity();
#endif // THINGSBOARD_ENABLE_DYNAMIC
            *position = *it;
            ++m_size;
        }
    }

    /// @brief Removes the element at the given position, has to move all element one to the left if the iterator does not point to the last valid element in the underlying data container
    /// @note The user is also cautioned that this function only erases the element, and that if the element is itself a pointer,
    /// the pointed-to memory is not touched in any way. Managing the pointer is the user's responsibility.
    /// Furthermore this method will assert if the position iterator is outside the range of this container,
    /// meaning if we are using an iterator to another container or if we passed an invalid iterator that would cause invalid memory access if dereferenced
    /// @tparam InputIterator Class that allows for forward incrementable access to data
    /// of the given data container, allows for using / passing either std::vector or std::array.
    /// See https://en.cppreference.com/w/cpp/iterator/input_iterator for more information on the requirements of the iterator
    /// @param position Iterator pointing to the element, that should be removed from the underlying data container
    void erase(const_pointer position) {
        assert(begin() >= position <= end());
        size_type const index = Helper::distance(begin(), position);
        // Move all elements after the index one position to the left
        for (size_type i = index; i <= m_size; ++i) {
            m_elements[i] = m_elements[i + 1];
        }
        // Decrease the size of the array to remove the last element, because either it was moved one index to the left or was the element we wanted to delete anyway
        --m_size;
    }

    /// @brief Returns a reference to the element at specified location index, with bounds checking
    /// @note If index is not within the range of the container, this method will assert and halt the application
    /// @param index Index of the element to return
    reference at(size_type const & index) {
        assert(index < m_size);
        return m_elements[index];
    }

    /// @copydoc Container::at
    const_reference at(size_type const & index) const {
        assert(index < m_size);
        return m_elements[index];
    }

    /// @brief Returns a reference to the element at specified location index. No bounds checking is performed
    /// @note Is more efficient but it is possible to read out of bounds data, which is undefined behaviour
    /// @param index Index of the element to return
    reference operator[](size_type index) {
        return m_elements[index];
    }

    /// @copydoc Container::operator[]
    const_reference operator[](size_type index) const {
        return m_elements[index];
    }

    /// @brief Erases all elements from the container. After this call, size() returns zero
    /// @note Simply sets the underlying size to 0, data will only be cleared in the destructor or if new data is inserted, which will overwrite old data.
    /// Therefore the user is also cautioned if the element is itself a pointer, the pointed-to memory is not touched in any way
    void clear() {
        m_size = 0;
    }

  private:
#if THINGSBOARD_ENABLE_DYNAMIC
    /// @brief Increases the internal capacity exponentially (current capacity * 2), if the underlying data container is currently full
    void increase_capacity() {
        if (m_size == m_capacity) {
            m_capacity = (m_capacity == 0) ? 1 : 2 * m_capacity;
            T* new_elements = new T[m_capacity]();
            if (m_elements != nullptr) {
                memcpy(new_elements, m_elements, m_size * sizeof(T));
                delete[] m_elements;
            }
            m_elements = new_elements;
        }
    }
#endif // THINGSBOARD_ENABLE_DYNAMIC

#if THINGSBOARD_ENABLE_DYNAMIC
    value_type *m_elements = {}; // Pointer to the start of our elements
    size_t     m_capacity = {};  // Allocated capacity that shows how many elements we could hold
#else
    value_type m_elements[Capacity] = {}; // Underlying c-array holding our data
#endif // THINGSBOARD_ENABLE_DYNAMIC
    size_type  m_size = {};               // Used size that shows how many elements we entered
};

#endif // Container_h
