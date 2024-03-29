#pragma once

#include <cstdint>

#include <mpi/core/mpi.hpp>

namespace mpi
{
enum class split_type : std::int32_t
{
  undefined         = MPI_UNDEFINED            ,
  shared            = MPI_COMM_TYPE_SHARED

#ifdef MPI_COMM_TYPE_HW_GUIDED
                                               ,
  hardware_guided   = MPI_COMM_TYPE_HW_GUIDED
#endif

#ifdef MPI_COMM_TYPE_HW_UNGUIDED
                                               ,
  hardware_unguided = MPI_COMM_TYPE_HW_UNGUIDED
#endif
};
}