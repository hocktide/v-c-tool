/**
 * \file include/vctool/components.h
 *
 * \brief Components enumeration.
 *
 * \copyright 2020 Velo Payments.  See License.txt for license terms.
 */

#ifndef VCTOOL_COMPONENTS_HEADER_GUARD
#define VCTOOL_COMPONENTS_HEADER_GUARD

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief vctool Components.
 */
enum vctool_components
{
    /**
     * \brief General Components.
     */
    VCTOOL_COMPONENT_GENERAL = 0x00U,

    /**
     * \brief file Component.
     */
    VCTOOL_COMPONENT_FILE = 0x01U,
};

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*VCTOOL_COMPONENTS_HEADER_GUARD*/