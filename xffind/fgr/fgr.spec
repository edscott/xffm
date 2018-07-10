# RPM spec file for SUSE OpenSUSE created by configure script.
Summary: Command line search tool	
Name: fgr
Version: 5.2.5
Release: 1
License: GPL-3.0
URL: http://xffm.org/fgr.html
Source0: %{name}-%{version}.tar.gz
Group: System/GUI/Other 
BuildRoot: %{_tmppath}/%{name}-root
Conflicts: rodent

%changelog
* Fri Nov 22 2013  <edscott@users.sf.net> 5.2.5-1
- RPM release


%description
Command line search tool, used by Rodent-fgr. May be used by any other program.
Practically no dependencies necessary, just a working grep program.


%prep
%setup -q

%build
%configure
make -j4
strip -s src/fgr

%clean
rm -rf $RPM_BUILD_ROOT

%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT mandir=%{_mandir}

%find_lang %{name}

%files  -f %{name}.lang
%defattr(-,root,root)
%doc AUTHORS COPYING README 
%{_mandir}/man1/fgr.1.gz

%{_bindir}/fgr


