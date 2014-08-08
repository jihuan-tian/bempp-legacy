from bempp.utils cimport shared_ptr

cdef extern from "bempp/grid/grid.hpp" namespace "Bempp":
    cdef cppclass c_Grid "Bempp::Grid":
        int dim() const
        int dimWorld() const
        int maxLevel() const
        int topology() const

    cdef enum Topology "Bempp::GridParameters::Topology":
        LINEAR "Bempp::GridParameters::LINEAR"
        TRIANGULAR "Bempp::GridParameters::TRIANGULAR"
        QUADRILATERAL "Bempp::GridParameters::QUADRILATERAL"
        HYBRID_2D "Bempp::GridParameters::HYBRID_2D"
        TETRAHEDRAL "Bempp::GridParameters::TRIANGULAR"

    cdef cppclass GridParameters:
        Topology topology

cdef class Grid:
    ## Holds pointer to C++ implementation
    cdef shared_ptr[c_Grid] impl_
    cdef void __create_from_file(self, dict kwargs,
            GridParameters &parameters) except *
    cdef void __create_cartesian_grid(self, dict kwargs,
            GridParameters &parameters) except *
