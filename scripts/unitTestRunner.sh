# The test `RealWorldExample_DAS_Matching` requires a file in the local
#    NMTools/test/resource directory. The test fails if run from the NMTools/build
#    directory because there is no NMTools/build/test/resources directory.
#    The first line of the test is the ASSERT that fails. We need the test to
#    pass so that it can execute all of its code for proper code coverage. We will
#    get around this issue by jumping back out of the build dir in order to 
#    execute the UnitTestRunner
#
cd ..
build/UnitTestRunner
cd -
