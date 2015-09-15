Summary: Loadable Kernel Module for generating rationalized kernel logs
Name: rational_printk
Version: 1.0.0
Release: 1
License: GPL
Group: System Environment/Base
URL: https://github.com/rtevans/rpk_modules
Source0: %{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root

%define debug_package %{nil}
%include rpm-dir.inc
#%define kernel 2.6.32-431.17.1.el6.x86_64 
%define kernel %(uname -r)
%define destdir /lib/modules/%{kernel}/extra

%description
This is a module that upon loading generates rationalized kernel
messages alongside standard kernel messages.  One can label
messages with a JobID by 

echo JOBID > /sys/module/rational_printk/parameters/JOBID 

before a job begins.  Note 

echo 0 > /sys/module/rational_printk/parameters/JOBID 

upon job completion is then necessary.

%prep
%setup -q

%build 
make 

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/%{destdir}
install -m 0755 %{name}.ko %{buildroot}/%{destdir}

%post
/sbin/depmod -ae %{kernel}
/sbin/modprobe rational_printk

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root)
%{destdir}/%{name}.ko

%preun
/sbin/modprobe -r rational_printk

%changelog
* Mon Mar  2 2015  <build@build.stampede.tacc.utexas.edu> - 
- Initial build.

