<?php include_once("zheader.php"); ?>

<h1>Name <img src="d_arr.png" /></h1>

<p class="pleft">libaccdb - The Unified Account Database</p>

<h1>libaccdb API <img src="d_arr.png" /></h1>

<p class="block"><tt>libaccdb</tt> provides some essential functions for
applications wishing to access a user database. It also contains some common
used code, such as password generation routines. Applications that wish to use
ACCDB must link against it, usually with gcc's <tt>-laccdb</tt> parameter.</p>

<div class="pleft2">
<table>
  <tr>
    <td class="cmd"><tt><b>#</b>include &lt;accdb.h&gt;<br />
<br />
<b>struct</b> ac_module <b>*</b>accdb_load(<b>const char *</b>module);<br />
<b>void</b> accdb_unload(<b>struct</b> ac_module <b>*</b>mp);</tt></td>
  </tr>
</table>
</div>

<p class="block"><tt>accdb_load()</tt> opens a back-end module, acquires all
necessary symbols and returns them in a <tt>malloc()</tt>ed <tt><b>struct</b>
ac_module</tt>, which is then to be freed with <tt>accdb_unload()</tt>.</p>

<p class="block">The <tt>module</tt> parameter can be:</p>

<ul>
  <li>The canonical name of the backend, e.g.
    <tt>shadow</tt>, which is then completed to <tt>accdb_shadow.so</tt> or
    <tt>accdb_shadow.dll</tt> (depending on OS).</li>

  <li>The full filename, e.g. <tt>accdb_shadow.so</tt>. Also possible with this
    is e.g. <tt>my_fancy_module.so</tt>, which would not be possible with the
    canonical approach, because <tt>accdb_</tt> would have been prepended.</li>

  <li>An asterisk (<tt>accdb_load("*");</tt>) to request the Default Backend,
    which is defined in <tt>./vetc/libaccdb/accdb</tt> or
    <tt>/etc/libaccdb/accdb</tt>, variable <tt>DEFAULT_BACKEND</tt>.</li>

</ul>

<p class="block">On failure, <tt>accdb_load()</tt> returns <tt>NULL</tt> and
sets <tt>errno</tt> to the ones set by system calls, plus <tt>EFAULT</tt> if no
<tt>b_open()</tt> function in the ACCDB module could be found. Errno will be
set to <tt>ENOENT</tt> if no default module was defined.</p>

<div class="pleft2">
<table>
  <tr>
    <td class="cmd"><tt><b>struct</b> ac_module <b>*</b>res = accdb_load("*", 0, <b>NULL</b>);<br />
res<b>-&gt;</b>open(res<b>-&gt;</b>state, 0, <b>NULL</b>);<br />
res<b>-&gt;</b>useradd(...);<br />
res<b>-&gt;</b>close(res<b>-&gt;</b>state);<br />
accdb_unload(res);</tt></td>
  </tr>
</table>
</div>

<p class="block">The back-end interface is discussed in <a
href="backend_api.php">its own chapter</a>.</p>

<h1>Password generation <img src="d_arr.png" /></h1>

<div class="pleft2">
<table>
  <tr>
    <td class="cmd"><tt><b>#</b>include &lt;accdb.h&gt;<br />
<br />
<b>int</b> vx_genpw(<b>char *</b>plain, <b>size_t</b> len, <b>unsigned long</b> flags);<br />
<b>int</b> vx_cryptpw(<b>const char *</b>key, <b>const char *</b>salt, <b>int</b> meth, <b>char **</b>crypted);</tt></td>
  </tr>
</table>
</div>

<p class="block"><tt>vx_genpw()</tt> generates a random password. It is
configured to use libHX's independent random generator layer, to use
<tt>/dev/urandom</tt> where possible (otherwise uses libc's <tt>rand()</tt>).
The new password of length <tt>len</tt> is put into <tt>plain</tt>. A trailing
<tt>'\0'</tt> character is appended, so <tt>plain</tt> must at least be 1
bigger than the value of <tt>len</tt>.</p>

<p class="block"><tt>flags</tt> is a bitmask, which may consists of these
options: <tt>GENPW_PHONEMIC</tt> uses a different algorithm, to choose
passwords which can be spoken and (may be | is) easy to remember.
<tt>GENPW_ONE_CASE</tt> specifies that there should be at least one upper-case
character in the plain password; <tt>GENPW_ONE_DIGIT</tt> is the same for a
digit, respectively.</p>

<p class="block"><tt>vx_cryptpw()</tt> takes a plain text <i>key</i> -- mostly
this a plaintext password -- and encrypts it, being usable for
<tt>/etc/shadow</tt> or similar. Space for the resulting crypted string is
allocated within <tt>vx_cryptpw()</tt>, and is put into <tt>*crypted</tt>. Be
sure to <tt>free()</tt> it when you are done with it. <tt>meth</tt> specifies
the encryption to use. Valid values are <tt>CRYPW_DES</tt>, <tt>CRYPW_MD5</tt>
and <tt>CRYPW_BLOWFISH</tt>. Blowfish is the preferred algorithm nowadays which
provides maximum security of these three.</p>

<p class="block">If <tt>salt</tt> is <tt>NULL</tt>, a new salt will be
generated. The <tt>salt</tt> parameter is usually only used for password
authentication.</p>

<div class="pleft2">
<table>
  <tr>
    <td class="cmd"><tt><b>char</b> pw[11], <b>*</b>cr;<br />
vx_genpw(pw, 10, GENPW_PHONEMIC | GENPW_ONE_DIGIT | GENPW_ONE_CASE);<br />
vx_cryptpw(pw, <b>NULL</b>, CRYPW_BLOWFISH, <b>&</b>cr);</tt></td>
  </tr>
</table>
</div>

<p class="block">DES and MD5 crypt is only available if your libc has them.
Blowfish encryption routines are always available since I have included them in
Vitalnix. So, Windows users only have Blowfish encryption at this time.</p>

<?php include_once("zfooter.php"); ?>
