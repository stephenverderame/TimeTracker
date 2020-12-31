#pragma once
#include <variant>
#include <functional>
namespace maybe {
	template<typename T>
	using Maybe = std::variant<T, std::monostate>;

	using Empty = std::monostate;

	template<typename T>
	T& getOrThrow(Maybe<T>& m)
	{
		if (std::get_if<T>(&m))
			return *std::get_if<T>(&m);
		throw std::runtime_error("No maybe value");
		return std::declval<T>();
	}
	template<typename T>
	const T& getOrThrow(const Maybe<T>& m)
	{
		if (std::get_if<T>(&m))
			return *std::get_if<T>(&m);
		throw std::runtime_error("No maybe value");
	}

	template<typename T>
	bool isPresent(const Maybe<T>& m)
	{
		return std::get_if<T>(&m);
	}

	template<typename T>
	void doIfPresent(Maybe<T>& m, std::function<void(T&)> f)
	{
		if (isPresent(m)) f(getOrThrow(m));
	}
}