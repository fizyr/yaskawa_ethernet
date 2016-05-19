#pragma once
#include <algorithm>
#include <cstdint>
#include <iterator>
#include <stdexcept>
#include <vector>

namespace dr {
namespace yaskawa {

template<typename T>
class array_view {
public:
	using value_type              = T;
	using size_type               = std::size_t;
	using difference_type         = std::ptrdiff_t;
	using reference               = T &;
	using const_reference         = T const &;
	using pointer                 = T *;
	using const_pointer           = T const *;
	using iterator                = pointer;
	using const_iterator          = const_pointer;
	using reverse_iterator        = std::reverse_iterator<iterator>;
	using const_reverse_iterator  = std::reverse_iterator<const_iterator>;

	using const_T    = const T;
	using nonconst_T = typename std::remove_const<T>::type;
	static constexpr bool is_const = std::is_const<T>::value;

	static constexpr size_type npos = size_type(-1);

private:
	/// Pointer to the beginning of the data.
	pointer data_;
	/// Number of elements in the view.
	size_type size_;

public:
	/// Construct an array view from a pointer and a size.
	array_view(pointer data, size_type size) : data_(data), size_(size) {}

	array_view(array_view<T> const & other) = default;
	array_view(array_view<T>      && other) = default;

	/// Construct from a vector of T.
	template<bool Enable = !is_const, typename = typename std::enable_if<Enable>::type>
	explicit array_view(std::vector<T> & other) : data_{other.data()}, size_{other.size()} {};

	/// Construct from a vector of non-const T.
	template<bool Enable = is_const, typename = typename std::enable_if<Enable>::type>
	explicit array_view(std::vector<T> const & other) : data_{other.data()}, size_{other.size()} {};

	/// Implicit conversion to array_view<T const>.
	operator array_view<T const> () noexcept {
		return {data(), size()};
	}

	reference       operator[] (size_type index)       noexcept { return data_[index]; }
	const_reference operator[] (size_type index) const noexcept { return data_[index]; }

	reference at(size_type index) {
		if (index >= size_) throw std::out_of_range("index out of range: " + std::to_string(index) + " not within [0, " + std::to_string(size_) + ")");
		return (*this)[index];
	}

	const_reference at(size_type index) const { return at(index); }

	reference       front()       noexcept { return data_[0]; }
	const_reference front() const noexcept { return data_[0]; }
	reference       back()        noexcept { return data_[size_ - 1]; }
	const_reference back()  const noexcept { return data_[size_ - 1]; }
	pointer         data()        noexcept { return data_; }
	const_pointer   data()  const noexcept { return data_; }

	iterator       begin()        noexcept { return data_; }
	const_iterator begin()  const noexcept { return data_; }
	const_iterator cbegin() const noexcept { return data_; }
	iterator       end()          noexcept { return data_ + size_; }
	const_iterator end()  const   noexcept { return data_ + size_; }
	const_iterator cend() const   noexcept { return data_ + size_; }

	std::size_t size() const noexcept { return size_;}
	constexpr std::size_t max_size() const noexcept { return size_type(-1) / sizeof(T); }
	bool empty() const noexcept { return size_ == 0; }

	void remove_front(size_type count) noexcept {
		data_ += count;
		size_ -= count;
	}

	void remove_back(size_type count) noexcept {
		size_ -= count;
	}

	array_view subview(size_type start = 0, size_type count = npos) {
		return array_view{data_ + start, std::min(count, size_ - start)};
	}

	void swap(array_view & other) {
		std::swap(this->data_, other.data_);
		std::swap(this->size_, other.size_);
	}

	std::vector<T> to_vector() const { return std::vector<T>{begin(), end()}; }
	operator std::vector<T>() const { return to_vector(); }
};

}}
