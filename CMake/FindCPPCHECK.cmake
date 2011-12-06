# cppcheck
#
#  Copyright (c) 2011 Mathieu Malaterre <mathieu.malaterre@gmail.com>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

FIND_PROGRAM(CPPCHECK_EXECUTABLE
  cppcheck
  )

MARK_AS_ADVANCED(
  CPPCHECK_EXECUTABLE
  )
