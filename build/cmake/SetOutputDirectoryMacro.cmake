###########################################################################
## $Id$
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: A macro to set CMake output directories.
##   SET_OUTPUT_DIRECTORY( dir [target1, target2, ...] )
##   When no targets are given, CMAKE_..._OUTPUT_DIRECTORY... variables
##   are set; when targets are given, ..._OUTPUT_DIRECTORY... properties
##   are set on the targets.

MACRO( SET_OUTPUT_DIRECTORY )

  SET( targets ${ARGN} )
  LIST( GET targets 0 dir )
  LIST( REMOVE_AT targets 0 )
  
  SET( products Runtime Library Archive )
  SET( configs Debug Release RelWithDebInfo MinSizeRel )
  
  FOREACH( product ${products} )
    STRING( TOUPPER "${product}" product )

    IF( targets )
      SET_TARGET_PROPERTIES( ${targets} PROPERTIES "${product}_OUTPUT_DIRECTORY" "${dir}" )
    ELSE()
      SET( "CMAKE_${product}_OUTPUT_DIRECTORY" "${dir}" )
    ENDIF()
    
    FOREACH( config ${configs} )
      STRING( TOUPPER "${config}" config )

      IF( targets )
        SET_TARGET_PROPERTIES( ${targets} PROPERTIES "${product}_OUTPUT_DIRECTORY_${config}" "${dir}" )
        FOREACH( target ${targets} )
          LIST( FIND BCITESTS ${target} idx )
          IF( NOT idx LESS  0 )
            BCI2000_ADD_BCITEST( ${target} )
          ENDIF()
        ENDFOREACH()
      ELSE()
        SET( "CMAKE_${product}_OUTPUT_DIRECTORY_${config}" "${dir}" )
      ENDIF()
      
    ENDFOREACH()
  ENDFOREACH()
  
ENDMACRO()
