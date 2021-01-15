Name:          StopWatch
Version:       %{version}
Release:       %{buildnumber}%{?dist}
Summary:       An implemnetation of simple timer class in C++
Group:         Development/Tools
License:       MIT
BuildRequires: cmake >= 2.8, gtest-devel >= 1.8.0, gtest >= 1.8.0
Requires:      dpiUser
ExclusiveArch: x86_64

%description

%prep
cd ~/rpmbuild/BUILD
rm -rf %{name}
mkdir -p %{name}
cd %{name}
tar xzf ~/rpmbuild/SOURCES/%{name}-%{version}.tar.gz
if [ $? -ne 0 ]; then
   exit $?
fi

%build
cd %{name}/
rm -f  CMakeCache.txt

if [ "%{buildtype}" == "-DUSE_LR_DEBUG=OFF"  ]; then
   /usr/bin/cmake -DVERSION:STRING=%{version}.%{buildnumber} \
      -DCMAKE_CXX_COMPILER_ARG1:STRING=' -std=c++14 -fPIC -Wall -Ofast -m64 -Wl,-rpath -Wl,. ' \
      -DCMAKE_BUILD_TYPE:STRING=Release -DBUILD_SHARED_LIBS:BOOL=ON
elif [ "%{buildtype}" == "-DUSE_LR_DEBUG=ON"  ]; then
   /usr/bin/cmake -DUSE_LR_DEBUG=ON -DVERSION:STRING=%{version}.%{buildnumber} \
      -DCMAKE_CXX_COMPILER_ARG1:STRING=' -std=c++14 -Wall -Werror -g -gdwarf-2 --coverage -O0 -fPIC -m64 -Wl,-rpath -Wl,.'
else
   echo "Unknown buildtype:" "%{buildtype}"
   exit 1
fi

make -j6
./UnitTestRunner
if [ "%{buildtype}" == "-DUSE_LR_DEBUG=ON"  ]; then
   sudo chmod +x /usr/local/probe/bin
   /usr/local/probe/bin/CodeCoverage.py
fi
mkdir -p $RPM_BUILD_ROOT/usr/local/probe/lib
cp -rfd lib%{name}.so* $RPM_BUILD_ROOT/usr/local/probe/lib
mkdir -p $RPM_BUILD_ROOT/usr/local/probe/include
cp src/*.h $RPM_BUILD_ROOT/usr/local/probe/include

%post

%preun

%postun

%files
%defattr(-,dpi,dpi,-)
/usr/local/probe/lib
/usr/local/probe/include
