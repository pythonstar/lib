# Some distributions name their Freetype 2 package "freetype", while others
# name it "freetype2".  You can define the name your distribution uses here.
%define freetype2 freetype

Summary:	A SWF output library
Name:		ming
Version:	0.4.3
Release:	1
License:	LGPL
Group:		System Environment/Libraries
Source:		http://prdownloads.sourceforge.net/ming/ming-%{version}.tar.gz
URL:            http://ming.sourceforge.net/
BuildRoot:	%{_tmppath}/%{name}-%{version}-root
BuildRequires:	%{freetype2}-devel zlib-devel giflib-devel libpng-devel
Requires:	%{freetype2} zlib giflib libpng

%description
Ming is a C library for generating SWF ("Flash") format movies, plus a set of wrappers for using the library from C++ and popular scripting languages like PHP, Perl, Python, and Ruby.

%package devel
Summary:	A SWF output library
Group:		Development/Libraries
Requires:	%{name} = %{version}
Requires:	%{freetype2}-devel zlib-devel giflib-devel libpng-devel

%description devel
The ming-devel package includes the static libraries, 
header files, and developer docs for the ming package.

Install ming-devel if you want to develop programs which 
will use ming.

%prep
%setup -q

%build
%configure 
%__make %{?_smp_mflags}

%install
[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT
%__make %{?_smp_mflags} \
    DESTDIR=$RPM_BUILD_ROOT \
    docdir=$RPM_BUILD_ROOT%{_docdir}/%{name} \
    pkgconfigdir=%{_libdir}/pkgconfig \
    install

%clean
[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr(-, root, root)
%doc README AUTHORS COPYING ChangeLog 
%{_libdir}/libming*.so.*
%{_bindir}/*
%{_mandir}/man1/*

%files devel
%defattr(-, root, root)
%{_includedir}/*
%{_libdir}/*.a
%{_libdir}/*.la
%{_libdir}/libming.so
%{_libdir}/pkgconfig
%{_mandir}/man3/*

%changelog
* Tue Sep 02 2006 John Ellson <ellson@research.att.com>
- Initial changelog entry