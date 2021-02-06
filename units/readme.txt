1.) change to the software checkout folder

    example: /home/'username'/marvin_base/trunk/software/units

2.) create a build directory

    example: > mkdir debug

3.) change to build directory

    example: cd debug

4.) setup the build-flow

    current example path is: /home/'username'/marvin_base/trunk/software/units/debug

    possible build-types are: DEBUG, RELEASE, LIB, COV

    example: cmake -DCMAKE_BUILD_TYPE=DEBUG ..

    Select ISP product line with parameter:ISP_VERSION
       unify_v1.2
         example: cmake -DCMAKE_BUILD_TYPE=DEBUG -DISP_VERSION=ISP8000L_V1801  ..
       unify_v1.3
         example: cmake -DCMAKE_BUILD_TYPE=DEBUG -DISP_VERSION=ISP8000L_V1801  -DSUBDEV_CHAR=1 ..
    Configure ISP software to ISP800L product line.

5.) start the build

    current example path is: /home/'username'/marvin_base/trunk/software/units/debug

    make help

    will print you the available make targets.

    examples:
    make                                 // compiles all modules
    make all                             // compiles all modules
    make -C template_module              // compiles template_modules lib/unit_test/test_app
    make template_modules_static_LINT    // runs lint on the static template library


6.) start the unit-tests

    current example path is: /home/'username'/marvin_base/trunk/software/units/debug

    example: make test



7.) with qt

    cmake -DCMAKE_BUILD_TYPE=DEBUG -DQT_QMAKE_EXECUTABLE=/usr/local/Trolltech/Qt-4.6.3/bin/qmake ..

8.) with qt and panasonic libs

    cmake -DCMAKE_BUILD_TYPE=RELEASE -DQT_QMAKE_EXECUTABLE=/usr/local/Trolltech/Qt-4.6.3/bin/qmake -DENABLE_PFID=YES ..

