//=======================================================================
// Copyright (c) 2013 Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
/**
 * @file k_median.hpp
 * @brief
 * @author Piotr Wygocki
 * @version 1.0
 * @date 2013-03-08
 */
#ifndef PAAL_K_MEDIAN_HPP
#define PAAL_K_MEDIAN_HPP

#include "facility_location_swap.h"
#include "facility_location_solution_adapter.h"
#include "facility_location.h"
#include "k_median_solution.h"

namespace paal {
namespace local_search {

/**
 * @class default_k_median_components
 * @brief Model of Multisearch_components with default multi search components
 * for k-median.
 */
using default_k_median_components =
    Multisearch_components<
        facility_locationget_moves_swap,
        facility_location_gain_swap,
        facility_location_commit_swap>;
}
}

#endif // PAAL_K_MEDIAN_HPP
