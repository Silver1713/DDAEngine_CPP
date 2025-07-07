#ifndef METRIC_DATA_HPP
#define METRIC_DATA_HPP

#include <deque>
#include <string>
#include "Enums.hpp"
#include "Metric.hpp"

// Check if the compiler supports C++20 concepts
#include <concepts>
template <typename T>
concept IsAveragable = requires(T a, T b) {
	{ a + b } -> std::convertible_to<T>;
	{ a / b } -> std::convertible_to<T>;
	{ a - b } -> std::convertible_to<T>;
	{ a* b } -> std::convertible_to<T>;
};


template <typename T>
struct TypeHack
{
	using type = T;
};


template <>
struct TypeHack<const char*>
{
	using type = std::string; // Specialization for string type
};

template <>
struct TypeHack<char*>
{
	using type = std::string; // Specialization for string type
};

template <typename T>
struct DataStore
{
	

	std::deque<T> data; // Using deque for efficient push_back and pop_front operations
	T lastValue; // Last value added to the data store
	bool useWindow = false; // Flag to indicate if a sliding window is used
	bool singleValue = false; // Flag to indicate if only a single value is stored
	size_t maxWindow = 30; // Maximum size of the sliding window



	void addData(const T& value);
	void clearData();
	void removeData(const T& value);


	// Metric Functions
	T average() const requires (IsAveragable<T>);

	T const& last() const;
	T& last();


	//Overload
	T& operator[](size_t index);
	T const& operator[](size_t index) const;


};

template <typename T>
T DataStore<T>::average() const requires (IsAveragable<T>)
{
	if (singleValue)
	{
		return lastValue; // If singleValue is true, return the last value directly
	}
	if (data.empty())
	{
		throw std::runtime_error("Cannot calculate average of empty data store.");
	}

	T sum = T(); // Initialize sum to zero
	for (const auto& value : data)
	{
		sum += value; // Accumulate the sum of all values
	}
	return sum / static_cast<T>(data.size()); // Return the average

}

template <typename T>
T& DataStore<T>::last()
{
	if (singleValue)
	{
		return lastValue; // Return the last value if singleValue is true
	}

	if (data.empty())
	{
		throw std::runtime_error("Data store is empty, cannot return last value.");
	}
	return data.back(); // Return the last element in the deque if singleValue is false
}

template <typename T>
T const& DataStore<T>::last() const
{
	if (singleValue)
	{
		return lastValue; // Return the last value if singleValue is true
	}

	if (data.empty())
	{
		throw std::runtime_error("Data store is empty, cannot return last value.");
	}
	return data.back(); // Return the last element in the deque if singleValue is false
}
template <typename T>
void DataStore<T>::addData(const T& value)
{
	if (useWindow && data.size() >= maxWindow)
	{
		data.pop_front(); // Remove the oldest element if the window size is exceeded
	}
	data.push_back(value); // Add the new value to the end of the deque

}

template <typename T>
void DataStore<T>::clearData()
{
	data.clear(); // Clear all data in the deque


}

template <typename T>
void DataStore<T>::removeData(const T& value)
{
	auto it = std::find(data.begin(), data.end(), value);
	if (it != data.end())
	{
		data.erase(it); // Remove the specified value from the deque
	}

}

template <typename T>
T& DataStore<T>::operator[](size_t index)
{
	if (index >= data.size())
	{
		throw std::out_of_range("Index out of range in DataStore.");
	}
	return data[index]; // Return the element at the specified index

}

template <typename T>
T const& DataStore<T>::operator[](size_t index) const
{
	if (index >= data.size())
	{
		throw std::out_of_range("Index out of range in DataStore.");
	}
	return data[index]; // Return the element at the specified index

}




template <typename T >
struct MetricData :  Metric
{
	MetricData() = default;
	MetricData(const std::string& id, DDAMetricType t, T val, bool bayesianInfer = false);
	using V = typename TypeHack<T>::type; // Use TypeHack to handle const char* and char* as std::string

	V value;
	DataStore<V> dataStore; // Data store for the metric values

	std::string ToString() const override;
	
	MetricDataTypes getValue() const override;

	void addValue(V val);


	template <typename U>
	requires (std::is_convertible_v<typename TypeHack<T>::type,U>)
	U as() const;
};

template <typename T>
MetricData<T>::MetricData(const std::string& id, DDAMetricType t, T val, bool bayesianInfer)
	: Metric(id, t, bayesianInfer), value(val)
{}


template <typename T>
Metric::MetricDataTypes MetricData<T>::getValue() const
{
	return value;

}



template <typename T>
template <typename U> requires (std::is_convertible_v<typename TypeHack<T>::type, U>)
U MetricData<T>::as() const
{
	return static_cast<U>(value); // Convert value to the specified type

}




template <typename T>
std::string MetricData<T>::ToString() const
{

	return std::to_string(value); // Convert value to string representation
}


template <>
inline std::string MetricData<const char*>::ToString() const
{
	return value; // For string type, return the value directly
}



#endif