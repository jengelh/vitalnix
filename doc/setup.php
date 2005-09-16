<?php include_once("zheader.php"); ?>

<h1>Overview <img src="d_arr.png" /></h1>

<p class="block">setup - Instructions on how to compile and install
Vitalnix</p>

<h1>Preface <img src="d_arr.png" /></h1>

<p class="block">There are three ways of getting Vitalnix installed. In the
following descriptions, replace <tt><b>%version</b></tt> and
<tt><b>%arch</b></tt> appropriately. But before we can get to the install
instructions, reading the next paragraph is obligatory.</p>

<h1>Prerequisites <img src="d_arr.png" /></h1>

<p class="block">Vitalnix depends on some libraries, popt, libHX and
libxml2.</p>

<div class="pleft">
<table border="1" class="bordered">
  <tr>
    <td class="t1"><p class="jblock"><i>popt</i> (at least v1.7)</p>

      <p class="jblock">"Popt is a C library for parsing command line
      parameters. Popt was heavily influenced by the <tt>getopt()</tt> and
      <tt>getopt_long()</tt> functions. It improves on them by allowing more
      powerful argument expansion. Popt can parse arbitrary <tt>argv[]</tt>
      style arrays and automatically set variables based on command line
      arguments.  Popt allows command line arguments to be aliased via
      configuration files and includes utility functions for parsing arbitrary
      strings into <tt>argv[]</tt> arrays using shell-like rules. &lt;Developed
      by RedHat&gt;"</p>

      <p class="jblock">It is usually packaged with your Linux distribution. If
      not, well, try <tt>redhat.com</tt> and grab <tt>rpm-4.1.tar.bz2</tt>.</p>

    </td>
  </tr>
  <tr>
    <td class="t2"><p class="jblock"><i>libHX</i> (at least 20050109)</p>

      <p class="jblock">It is an all-purpose library, providing commonly used
      functions not available in standard libc's as well as OS-abstracted
      functions for various things.</p>

      <p class="jblock">The current version can be found in <a
      href="http://linux01.org:2222/prog-libHX.php"
      target="_blank">http://linux01.org:2222/prog-libHX.php</a> (If you do not
      install it system-wide, put <tt>libHX.h</tt> and <tt>libHX.so*</tt> in
      the root of the vitalnix hierarchy.</p>

    </td>
  </tr>
  <tr>
    <td class="t1"><p class="jblock"><i>libxml2</i> (at least v2.5.3)</p>

      <p class="jblock">It is a library for parsing etc. of XML files. You can
      find it at <a href="http://xmlsoft.org/"
      target="_blank">http://xmlsoft.org/</a>. You will need it only for
      C-spark.</p>

    </td>
  </tr>
  <tr>
    <td class="t2"><p class="jblock"><i>libintl</i> (MinGW only)</p>

      <p class="jblock">Used by <tt>popt1.dll</tt> from <a
      href="http://gnuwin32.sourceforge.net/" target="_blank">GNUwin32</a>.
      (The reason you do not need this under Linux is because libintl is
      compiled into <tt>libc.so.6</tt>.)</p>

    </td>
  </tr>
  <tr>
    <td class="t1"><p class="jblock"><i>libiconv</i> (MinGW only)</p>

      <p class="jblock">Used by libintl from <a
      href="http://gnuwin32.sourceforge.net/" target="_blank">GNUwin32</a>.
      (The reason you do not need this under Linux is because libiconv is
      compiled into <tt>libc.so.6</tt>.)</p>

    </td>
  </tr>
</table>
</div>

<h1>Installing <img src="d_arr.png" /></h1>

<h2>From precompiled RPM</h2>

<p class="block">One is to install the precompiled RPM package; I recommend
this for anyone just wanting to get it run.</p>

<div class="pleft2">
<table>
  <tr>
    <td class="cmd"><tt><b>#</b> rpm -Uhv
      vitalnix-<b>%version</b>.<b>%arch</b>.rpm</tt></td>
  </tr>
</table>
</div>

<p class="block">This single command installs the RPM. Unless <tt>rpm</tt>
returns some error, this is all there is to do. Precompiled RPM packages for
<tt>i586-pc-gnu-linux</tt> and <tt>i686-pc-gnu-linux</tt> can be found on the
Vitalnix Homepage. If, for some reason this does not work, i.e. because you
have a different shared library versions than I do, I suggest reverting to RPM
from specfile (see below).</p>

<p class="block">Program files will be installed in these paths:</p>

<div class="pleft">
<table border="1" class="bordered">
  <tr>
    <td class="t1">libaccdb and ACCDB modules</td>
    <td class="t1"><tt>/usr/lib/</tt></td>
  </tr>
  <tr>
    <td class="t2">Configuration files for libaccdb</td>
    <td class="t2"><tt>/etc/libaccdb/</tt></td>
  </tr>
  <tr>
    <td class="t1">C/C++ developer files for libaccdb</td>
    <td class="t1"><tt>/usr/include/</tt></td>
  </tr>
  <tr>
    <td class="t2">Administrative programs (vuseradd, etc.)</td>
    <td class="t2"><tt>/usr/sbin/</tt></td>
  </tr>
  <tr>
    <td class="t1">Configuration files for administrative programs</td>
    <td class="t1"><tt>/etc/</tt></td>
  <tr>
    <td class="t2">Vitalnix supply programs (<tt>mkgroupdir</tt>,
      <tt>sdf2xml</tt>, etc.)</td>
    <td class="t2"><tt>/usr/lib/vitalnix/</tt></td>
  </tr>
  <tr>
    <td class="t1">C-spark binary</td>
    <td class="t1"><tt>/usr/sbin/</tt></td>
  </tr>
  <tr>
    <td class="t2">Configuration file for Spark</td>
    <td class="t2"><tt>/etc/spark.conf</tt></td>
  </tr>
  <tr>
    <td class="t1">Runtime data (which is your XML input files)</td>
    <td class="t1"><tt>/var/lib/vitalnix/</tt></td>
  </tr>
  <tr>
    <td class="t2">Documentation (only installed by RPM package)</td>
    <td class="t2"><tt>/usr/share/doc/packages/</tt></td>
  </tr>
</table>
</div>

<h2>From precompiled ZIP</h2>

<p class="block">Speaks for itself and applies to Win32 only. There is no real
need to install ("fill up the registry") it, just extract the package into a
new folder and run the applications from there.</p>

<h2>From source</h2>

<p class="block">The second way is to compile from source. It is for those who
want to keep it in a single directory. However, if you intend to install it
system-wide after compilation, I still point to the RPM specfile.</p>

<div class="pleft2">
<table>
  <tr>
    <td class="cmd"><tt>
      <b>$</b> tar -jvxf vitalnix-<b>%version</b>.tbz2;<br />
      <b>$</b> cd vitalnix-<b>%version</b>;<br />
      <b>$</b> ./Makefile.RUN;
    </tt></td>
  </tr>
</table>
</div>

<p class="block">Extract the source tree and run <tt>Makefile.RUN</tt>. (You
may add any <tt>GNU make</tt> relevant flags, like <tt>-j2</tt>.) If everything
went well, ACCDB, its back-ends, system programs and C-spark will be compiled,
lying in the current directory (the root of the Vitalnix tree). Note that
"<tt>.</tt>", the current directory, is tried first for all libraries (this is
the default under Win32 anyway), so if you happen to run any of the utilities
from a directory which has a different, say, <tt>libaccdb.so.0.1</tt>, that one
would be used instead.</p>

<p class="block"><tt>/etc</tt>, <tt>/usr/local/etc</tt> and <tt>.</tt> as
search paths for configuration files are hardcoded into the programs.</p>

<p class="block">There is no <tt>install</tt> target in the Makefile to
encourage the use of some database (like RPM, but you can use any other too) to
track what files are installed. It is just time-consuming to keep the
<tt>install</tt> (even <tt>uninstall</tt>) target up-to-date with the RPM/xxx
specfiles.</p>

<h2>With RPM specfile</h2>

<p class="block">The third method, is to compile from source using the spec
file for RPM. It is better than method #2 (source tree), as RPM will keep track
of what files were installed. That info would be lost when using
<tt>gmake</tt>'s <tt>install</tt>. You will also be interested in this one when
there is no precompiled RPM for your architecture and/or target OS. Especially
because it combines the best of both worlds: machine-dependent flags and
optimizations (will require adjustments in the <tt>Makefile</tt> or specfile)
<i>and</i> file tracking.</p>

<div class="pleft2">
<table>
  <tr>
    <td class="cmd"><tt>
      <b>$</b> tar -jxf vitalnix-<b>%version</b>.tbz2;<br />
      <b>$</b> cd vitalnix-<b>%version</b>;<br />
      <b>$</b> ./Makefile.RUN rpm;
    </tt></td>
  </tr>
</table>
</div>

<p class="block">You can then install the RPM found in
<tt><b>$RPMBUILD_PATH</b>/RPMS/<b>%arch</b>/</tt>. For SuSE, this would be
<tt>/usr/src/packages</tt>, for RedHat with <tt>/usr/src/redhat</tt>, other
distributinos may offer it in another directory.</p>

<h2>From Source RPM</h2>

<p class="block">I do not hand out any source RPMs, but should you run across
one, you can use:</p>

<div class="pleft2">
<table>
  <tr>
    <td class="cmd"><tt><b>$</b> rpm --rebuild vitalnix.srpm;</td>
  </tr>
</table>
</div>

<h1>Various flags <img src="d_arr.png" /></h1>

<p class="block">You can pass <tt>DEBUG=1</tt> to <tt>gmake</tt> to enable
debugging support for GDB. (Note: The <tt>Makefile</tt>s in CVS have
<tt>DEBUG</tt> set to 1, the released packages <b>DO NOT</b>.)</p>

<p class="block">As of Vitalnix v1.0.26.0, the make system has been silenced à
la Linux v2.6.0. To get the usual make verbosity with printing all the commands
that are executed, pass <tt>V=1</tt>.</p>

<?php include_once("zfooter.php"); ?>
