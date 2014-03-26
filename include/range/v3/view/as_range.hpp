//  Copyright Eric Niebler 2014
//
//  Use, modification and distribution is subject to the
//  Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// For more information, see http://www.boost.org/libs/range/
//
#ifndef RANGES_V3_VIEW_AS_RANGE_HPP
#define RANGES_V3_VIEW_AS_RANGE_HPP

#include <type_traits>
#include <range/v3/range_fwd.hpp>
#include <range/v3/size.hpp>
#include <range/v3/begin_end.hpp>
#include <range/v3/range_traits.hpp>
#include <range/v3/range_concepts.hpp>
#include <range/v3/range_facade.hpp>
#include <range/v3/iterator_range.hpp>
#include <range/v3/utility/bindable.hpp>
#include <range/v3/utility/iterator_concepts.hpp>

namespace ranges
{
    inline namespace v3
    {
        template<typename Iterable>
        struct as_range_view
          : range_facade<as_range_view<Iterable>, is_infinite<Iterable>::value>
        {
        private:
            friend range_core_access;
            Iterable rng_;

            struct cursor
            {
            private:
                using base_iterator_t = range_iterator_t<Iterable const>;
                using base_sentinel_t = range_sentinel_t<Iterable const>;

                base_iterator_t it_;
                base_sentinel_t se_;
                bool is_sentinel_;

                void clean()
                {
                    if(is_sentinel_)
                    {
                        while(it_ != se_)
                            ++it_;
                        is_sentinel_ = false;
                    }
                }
            public:
                cursor() = default;
                cursor(base_iterator_t it, base_sentinel_t se, bool is_sentinel)
                  : it_(std::move(it)), se_(std::move(se)), is_sentinel_(is_sentinel)
                {}
                auto current() const -> decltype(*it_)
                {
                    RANGES_ASSERT(!is_sentinel_ && it_ != se_);
                    return *it_;
                }
                bool equal(cursor const &that) const
                {
                    return is_sentinel_ ?
                        that.is_sentinel_ || that.it_ == se_ :
                        that.is_sentinel_ ?
                            it_ == that.se_ :
                            it_ == that.it_;
                }
                void next()
                {
                    RANGES_ASSERT(!is_sentinel_ && it_ != se_);
                    ++it_;
                }
                CONCEPT_REQUIRES(BidirectionalIterator<base_iterator_t>())
                void prev()
                {
                    clean();
                    --it_;
                }
                CONCEPT_REQUIRES(RandomAccessIterator<base_iterator_t>())
                void advance(range_difference_t<Iterable> n)
                {
                    clean();
                    it_ += n;
                }
                CONCEPT_REQUIRES(RandomAccessIterator<base_iterator_t>())
                range_difference_t<Iterable> distance_to(cursor const &that) const
                {
                    clean();
                    that.clean();
                    return that.it_ - it_;
                }
            };
            cursor begin_cursor() const
            {
                return {ranges::begin(rng_), ranges::end(rng_), false};
            }
            cursor end_cursor() const
            {
                return {ranges::begin(rng_), ranges::end(rng_), true};
            }
        public:
            explicit as_range_view(Iterable && rng)
              : rng_(std::forward<Iterable>(rng))
            {}
            CONCEPT_REQUIRES(SizedIterable<Iterable>())
            range_size_t<Iterable> size() const
            {
                return ranges::size(rng_);
            }
        };

        namespace view
        {
            struct as_ranger : bindable<as_ranger>, pipeable<as_ranger>
            {
                template<typename InputIterable>
                static as_range_view<InputIterable>
                invoke(as_ranger, InputIterable && rng)
                {
                    CONCEPT_ASSERT(ranges::Iterable<InputIterable>());
                    CONCEPT_ASSERT(ranges::InputIterator<range_iterator_t<InputIterable>>());
                    CONCEPT_ASSERT(!ranges::Range<InputIterable>());
                    return as_range_view<InputIterable>{std::forward<InputIterable>(rng)};
                }
            };

            RANGES_CONSTEXPR as_ranger as_range{};
        }
    }
}

#endif
