//=======================================================================
// Copyright (c)
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file lp_row_generation.hpp
 * @brief
 * @author Piotr Godlewski, Robert Rosołek
 * @version 1.0
 * @date 2013-06-04
 */
#ifndef PAAL_LP_ROW_GENERATION_HPP
#define PAAL_LP_ROW_GENERATION_HPP

#include "lp_base.h"
#include "problem_type.h"
#include "rotate.h"

#include <boost/range/counting_range.hpp>

namespace paal {
namespace lp {

/**
 * Finds an extreme point solution to the LP using row generation:
 * solves the initial LP and then ask the separation oracle if the found
 * solution is a feasible solution to the complete problem. If not,
 * adds a new row (generated by the oracle) to the LP and re-optimizes it.
 * This procedure is iterated until a feasible solution to the full LP
 * is found.
 */
template <class TryAddViolated, class SolveLp>
    problem_type row_generation(TryAddViolated try_add_violated, SolveLp solve_lp)
    {
        problem_type res;
        do res = solve_lp(); while (res == OPTIMAL && try_add_violated());
        return res;
    }

/**
 * @brief functor for adding maximum violated constraint
 *
 * @tparam GetCandidates
 * @tparam HowViolated
 * @tparam AddViolated
 * @tparam CompareHow
 */
template<
    class GetCandidates,
    class HowViolated,
    class AddViolated,
    class CompareHow
>
class add_max_violated {
   GetCandidates m_get_candidates;
   HowViolated m_how_violated;
   AddViolated m_add_violated;
   CompareHow m_cmp;

   public:
      ///contructor
      add_max_violated(GetCandidates get_candidates,
            HowViolated how_violated, AddViolated add_violated, CompareHow cmp)
         : m_get_candidates(get_candidates), m_how_violated(how_violated),
            m_add_violated(add_violated), m_cmp(cmp) {}

      ///operator()
      bool operator()() {
         auto&& cands = m_get_candidates();
         using how_violated_t = puretype(m_how_violated(*std::begin(cands)));
         using cand_it_t = puretype(std::begin(cands));
         boost::optional<std::pair<how_violated_t, cand_it_t>> most;
         for (auto cand : boost::counting_range(cands)) {
            auto const how = m_how_violated(*cand);
            if (!how) continue;
            if (!most || m_cmp(most->first, how))
               most = std::make_pair(std::move(how), cand);
         }
         if (!most) return false;
         m_add_violated(*most->second);
         return true;
      }
};

///functor computing add_max_violated
struct max_violated_separation_oracle {
   template <
      class GetCandidates,
      class HowViolated,
      class AddViolated,
      class CompareHow = utils::less
   >
   ///operator()
   auto operator()(
      GetCandidates get_candidates,
      HowViolated is_violated,
      AddViolated add_violated,
      CompareHow compare_how = CompareHow{}
   ) const {
      return add_max_violated<GetCandidates, HowViolated, AddViolated,
         CompareHow>(get_candidates, is_violated, add_violated, compare_how);
   }
};

///functor
template <class GetCandidates,
          class HowViolated,
          class AddViolated,
          class ReorderCandidates>
class add_first_violated {
   GetCandidates m_get_candidates;
   HowViolated m_how_violated;
   AddViolated m_add_violated;
   ReorderCandidates m_reorder_candidates;

   public:
      ///constructor
      add_first_violated(
         GetCandidates get_candidates,
         HowViolated how_violated,
         AddViolated add_violated,
         ReorderCandidates reorder_candidates
      ) : m_get_candidates(get_candidates),
         m_how_violated(how_violated),
         m_add_violated(add_violated),
         m_reorder_candidates(std::move(reorder_candidates)) {}

      ///operator()
      bool operator()() {
         auto&& cands = m_get_candidates();
         auto reordered =
            m_reorder_candidates(std::forward<decltype(cands)>(cands));
         for (auto c : boost::counting_range(reordered)) {
            if (m_how_violated(*c)) {
                m_add_violated(*c);
                return true;
            }
         }
         return false;
      }
};

///functor computing add_first_violated
struct first_violated_separation_oracle {
   template <
      class GetCandidates,
      class HowViolated,
      class AddViolated,
      class ReorderCandidates = utils::identity_functor
   >
   ///operator()
   auto operator() (
      GetCandidates get_candidates,
      HowViolated how_violated,
      AddViolated add_violated,
      ReorderCandidates reorder_candidates = ReorderCandidates{}
   ) const {
      return add_first_violated<GetCandidates, HowViolated, AddViolated,
         ReorderCandidates>(get_candidates, how_violated, add_violated,
            reorder_candidates);
   }
};

namespace detail {
template <class URNG>
class random_rotate {
   URNG m_g;
   public:
      random_rotate(URNG&& g)
         : m_g(std::forward<URNG>(g)) {}
      template <class ForwardRange>
      auto operator()(const ForwardRange& rng)
      {
         auto const len = boost::distance(rng);
         std::uniform_int_distribution<decltype(len)> d(0, len);
         return utils::rotate(rng, d(m_g));
      }
};

template <class URNG = std::default_random_engine>
auto make_random_rotate(URNG&& g = URNG{})
{
   return random_rotate<URNG>(std::forward<URNG>(g));
}
} //! detail

///functor returning add_first_violated
///Separation oracle for the row generation,
///using the random violated strategy.
struct random_violated_separation_oracle {
   template <
      class GetCandidates,
      class HowViolated,
      class AddViolated,
      class URNG = std::default_random_engine
   >
   ///operator()
   auto operator() (
      GetCandidates get_candidates,
      HowViolated how_violated,
      AddViolated add_violated,
      URNG&& g = URNG{}
   ) const {
      return first_violated_separation_oracle{}(get_candidates,
            how_violated, add_violated, detail::make_random_rotate(std::forward<URNG>(g)));
   }
};


} // lp
} // paal

#endif // PAAL_LP_ROW_GENERATION_HPP
