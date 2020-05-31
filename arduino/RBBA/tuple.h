#ifndef TUPLE_H
#define TUPLE_H


// tuple implementation from https://eli.thegreenplace.net/2014/variadic-templates-in-c/

namespace tpl {

// helper
template<bool B, class T = void>
struct enable_if {};
 
template<class T>
struct enable_if<true, T> { typedef T type; };



// basic tuple
template <class... Ts> struct tuple {};

template <class T, class... Ts>
struct tuple<T, Ts...> : tuple<Ts...> {
  tuple(T t, Ts... ts) : tuple<Ts...>(ts...), tail(t) {}

  T tail;
};


template <uint8_t, class> struct elem_type_holder;

template <class T, class... Ts>
struct elem_type_holder<0, tuple<T, Ts...>> {
  typedef T type;
};

template <uint8_t k, class T, class... Ts>
struct elem_type_holder<k, tuple<T, Ts...>> {
  typedef typename elem_type_holder<k - 1, tuple<Ts...>>::type type;
};


// basic get
template <uint8_t k, class... Ts>
typename enable_if<
    k == 0, typename elem_type_holder<0, tuple<Ts...>>::type&>::type
get(tuple<Ts...>& t) {
  return t.tail;
}

template <uint8_t k, class T, class... Ts>
typename enable_if<
    k != 0, typename elem_type_holder<k, tuple<T, Ts...>>::type&>::type
get(tuple<T, Ts...>& t) {
  tuple<Ts...>& base = t;
  return get<k - 1>(base);
}


// a version of get that can be parameterized with an idx, if you know the type of the entry
// inspired by https://stackoverflow.com/questions/36612596/tuple-to-parameter-pack
// sadly we can't use fold expressions ... only c++11 here in arduinoland

template<uint8_t ...> struct idx_seq {};

template<uint8_t N, uint8_t ...S> struct idx_seq_gens : idx_seq_gens<N - 1, N - 1, S...> { };

template<uint8_t... S> struct idx_seq_gens<0, S...>{ typedef idx_seq<S...> type; };

template<typename TupleElement, typename Tuple, uint8_t... S>
TupleElement* get_elem(uint8_t idx, Tuple &t, idx_seq<S...>) {
  return nullptr;
}

template<typename TupleElement, typename Tuple, uint8_t S1, uint8_t... S>
TupleElement* get_elem(uint8_t idx, Tuple &t, idx_seq<S1, S...>) {
  if (idx == S1) {
    return static_cast<TupleElement*>(&(get<S1>(t)));
  }
  return get_elem<TupleElement>(idx,t,idx_seq<S...>());
}

template<typename TupleElement, class... Ts>
TupleElement& get(uint8_t idx, tuple<Ts...>& t) {
  return *(get_elem<TupleElement>(idx,t,typename idx_seq_gens<sizeof...(Ts)>::type()));
}

template<typename TupleElement, class... Ts>
TupleElement* get_p(uint8_t idx, tuple<Ts...>& t) {
  return get_elem<TupleElement>(idx,t,typename idx_seq_gens<sizeof...(Ts)>::type());
}

}

#endif
