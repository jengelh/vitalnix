<?php include_once("Base-header.php"); ?>

<h1>General</h1>

<p class="code"><code><b>#</b>include &lt;vitanlix/libvxdb/libvxdb.h&gt;<br />
<br />
<b>struct</b> vxdb_user;<br />
<b>struct</b> vxdb_group;<br />
<b>struct</b> vxdb_state;</code></p>

<h1>Function overview</h1>

<p class="code"><code><b>#</b>include &lt;vitalnix/libvxdb/libvxdb.h&gt;<br />
<br />
<b>struct</b> vxdb_state <b>*</b>vxdb_load(<b>const char *</b>module);<br />
<b>void</b> vxdb_unload(<b>struct</b> vxdb_state <b>*</b>state);
</code></p>

<h2>vxdb_load</h2>

<p class="block">The <code>vxdb_load()</code> function loads a VXDB (user
account database back-end driver, acquires and fixes up all symbols and returns
a struct describing the loaded module. The <code>module</code> parameter can be
one of:</p>

<ul>
  <li>the magic string <code>"*"</code> which will select the default module as
    listed in <code>/etc/vitalnix/libvxdb.conf</code></li>
  <li>the filename of the module, e.g. <code>"libvxdb_shadow.so"</code></li>
  <li>the canonical name of the module, e.g. <code>"mysql"</code> (which is
    completed to <code>"libvxdb_mysql.so"</code>)</li>
</ul>

<p class="block">The second way is not widely used, because it the filename
extension of the shared library depends on the architecture Vitalnix runs on.
Although the third point only lists <code>.so</code> as an example, the
<code>.dll</code> extension is also tried when completing to a filename, even
under non-Win32. Note that for any module which is not named in the fashion of
<code>libvxdb_*.so</code>, there is only the second way.</p>

<p class="block">On failure, <code>vxdb_load()</code> will return
<code>NULL</code> and <code>errno</code> will be set.</p>

<h2>vxdb_unload</h2>

<p class="block">Unloads the VXDB module.</p>

<h1>Function overview&nbsp;-- Module access functions</h1>

<p class="code"><code><b>#</b>include &lt;vitalnix/libvxdb/xafunc.h&gt;<br />
<br />
<b>int</b> vxdb_open(<b>struct</b> vxdb_state <b>*</b>state, <b>long</b> flags);<br />
<b>void</b> vxdb_close(<b>struct</b> vxdb_state <b>*</b>state);<br />
<b>int</b> vxdb_modctl(<b>struct</b> vxdb_state <b>*</b>state, <b>long</b> command, <b>...</b>);<br />
<b>int</b> vxdb_lock(<b>struct</b> vxdb_state <b>*</b>state);<br />
<b>int</b> vxdb_unlock(<b>struct</b> vxdb_state <b>*</b>state);<br />
<b>int</b> vxdb_useradd(<b>struct</b> vxdb_state <b>*</b>state, <b>const struct</b> vxdb_user <b>*</b>user);<br />
<b>int</b> vxdb_usermod(<b>struct</b> vxdb_state <b>*</b>state, <b>const struct</b> vxdb_user <b>*</b>search_mask, <b>const struct</b> vxdb_user <b>*</b>replace_mask);<br />
<b>int</b> vxdb_userdel(<b>struct</b> vxdb_state <b>*</b>state, <b>const struct</b> vxdb_user <b>*</b>search_mask);<br />
<b>int</b> vxdb_userinfo(<b>struct</b> vxdb_state <b>*</b>state, <b>const struct</b> vxdb_user <b>*</b>search_mask, <b>struct</b> vxdb_user <b>*</b>result, <b>size_t</b> size);<br />
<b>void *</b>vxdb_usertrav_init(<b>struct</b> vxdb_state <b>*</b>state);<br />
<b>int</b> vxdb_usertrav_walk(<b>struct</b> vxdb_state <b>*</b>state, <b>void *</b>data, <b>struct</b> vxdb_user <b>*</b>result);<br />
<b>void</b> vxdb_usertrav_free(<b>struct</b> vxdb_state <b>*</b>state, <b>void *</b>data);<br />
<b>int</b> vxdb_groupadd(<b>struct</b> vxdb_state <b>*</b>state, <b>const struct</b> vxdb_group <b>*</b>group);<br />
<b>int</b> vxdb_groupmod(<b>struct</b> vxdb_state <b>*</b>state, <b>const struct</b> vxdb_group <b>*</b>search_mask, <b>const struct</b> vxdb_group <b>*</b>replace_mask);<br />
<b>int</b> vxdb_groupdel(<b>struct</b> vxdb_state <b>*</b>state, <b>const struct</b> vxdb_group <b>*</b>search_mask);<br />
<b>int</b> vxdb_groupinfo(<b>struct</b> vxdb_state <b>*</b>state, <b>const struct</b> vxdb_group <b>*</b>search_mask, <b>struct</b> vxdb_group <b>*</b>result, <b>size_t</b> size);<br />
<b>void *</b>vxdb_grouptrav_init(<b>struct</b> vxdb_state <b>*</b>state);<br />
<b>int</b> vxdb_grouptrav_walk(<b>struct</b> vxdb_state <b>*</b>state, <b>void *</b>data, <b>struct</b> vxdb_group <b>*</b>result);<br />
<b>void</b> vxdb_grouptrav_free(<b>struct</b> vxdb_state <b>*</b>state, <b>void *</b>data);
</code></p>

<h2>vxdb_open</h2>

<p class="block">Open the password database associated with state using
<code>flags</code>. <code>state</code> must have been obtained using
<code>vxdb_load()</code> before.</p>

<h2>vxdb_close</h2>

<h2>vxdb_lock</h2>

<p class="block">Lock the database. Returns an AEE code. If the database does
not implement it, either zero or <code>-ENOSYS</code> is returned.</p>

<h2>vxdb_unlock</h2>

<p class="block">Unlock the database. Always succeeds (it really depends on the
VXDB module&nbsp;-- and returns an AEE code).</p>

<h2>vxdb_useradd</h2>

<p class="block">Returns <code>-EINVAL</code> if the username is empty, or
<code>-EEXIST</code> if the username already exists. It is
implementation-defined whether the creation of accounts with non-unique UID is
allowed. If forbidden, <code>-EEXIST</code> is returned too.</p>

<h2>vxdb_usermod</h2>

<p class="block">Returns <code>-EINVAL</code> if both username and uid in the
search mask are empty, or returns <code>-ENOENT</code> if the user was not
found.</p>

<h2>vxdb_userdel</h2>

<p class="block">Returns <code>-EINVAL</code> if both username and uid in the
search mask are empty, or returns <code>-ENOENT</code> if the user was not
found.</p>

<h1>Function overview&nbsp;-- Extra access functions</h1>

<p class="code"><code><b>#</b>include &lt;vitalnix/libvxdb/xwfunc.h&gt;<br />
<br />
<b>int</b> vxdb_getpwnam(<b>struct</b> vxdb_state <b>*</b>state, <b>const char *</b>user, <b>struct</b> vxdb_user <b>*</b>result);<br />
<b>int</b> vxdb_getpwuid(<b>struct</b> vxdb_state <b>*</b>state, <b>unsigned int</b> uid, <b>struct</b> vxdb_user <b>*</b>result);<br />
<b>int</b> vxdb_getgrnam(<b>struct</b> vxdb_state <b>*</b>state, <b>const char *</b>group, <b>struct</b> vxdb_group <b>*</b>result);<br />
<b>int</b> vxdb_getgrgid(<b>struct</b> vxdb_state <b>*</b>state, <b>unsigned int</b> gid, <b>struct</b> vxdb_group <b>*</b>result);
</code></p>

<p class="block">These functions provide simple wrappers around
<code>vxdb_userinfo()</code> and <code>vxdb_groupinfo()</code> that take the
burden off the user to create the search mask beforehand if one only wants to
search for one paramter (in this case either name or uid/gid). Just as their
underlying function, they will all allocate the internal data buffer in the
<code>struct vxdb_user</code>/<code>vxdb_group</code> if necessary.</p>

<h1>Auxiliary functions</h1>

<p class="code"><code><b>#</b>include &lt;vitalnix/libvxdb/libvxdb.h&gt;<br />
<br />
<b>void</b> vxdb_user_clean(<b>struct</b> vxdb_user <b>*</b>user);<br />
<b>void</b> vxdb_user_copy(<b>struct</b> vxdb_user <b>*</b>dest, <b>const struct</b> vxdb_user <b>*</b>src);<br />
<b>struct</b> vxdb_user <b>*</b>vxdb_user_dup(<b>const struct</b> vxdb_user <b>*</b>src);<br />
<b>void</b> vxdb_user_free(<b>struct</b> vxdb_user <b>*</b>user, <b>int</b> heap);<br />
<b>void</b> vxdb_user_nomodify(<b>struct</b> vxdb_user <b>*</b>user);<br />
<br />
<b>void</b> vxdb_group_clean(<b>struct</b> vxdb_group <b>*</b>group);<br />
<b>void</b> vxdb_group_copy(<b>struct</b> vxdb_group <b>*</b>dest, <b>const struct</b> vxdb_group <b>*</b>src);<br />
<b>struct</b> vxdb_user <b>*</b>vxdb_group_dup(<b>const struct</b> vxdb_group <b>*</b>src);<br />
<b>void</b> vxdb_group_free(<b>struct</b> vxdb_user <b>*</b>user, <b>bool</b> heap);<br />
<b>void</b> vxdb_group_nomodify(<b>struct</b> vxdb_group <b>*</b>group);</code></p>

<h2>vxdb_user_clean</h2>

<p class="block">Fills the user structure pointed to by <code>user</code> with
the defaults, which are:</p>

<table border="1">
  <tr>
    <td class="t1"><code>-&gt;pw_name</code></td>
    <td class="t1"><code>NULL</code></td>
  </tr>
  <tr>
    <td class="t2"><code>-&gt;pw_uid</code></td>
    <td class="t2"><code>VXDB_NOUID</code></td>
  </tr>
  <tr>
    <td class="t1"><code>-&gt;pw_gid</code></td>
    <td class="t1"><code>VXDB_NOGID</code></td>
  </tr>
  <tr>
    <td class="t2"><code>-&gt;pw_igrp</code> (NOT IMPLEMENTED YET)</td>
    <td class="t2"><code>NULL</code></td>
  </tr>
  <tr>
    <td class="t1"><code>-&gt;pw_real</code></td>
    <td class="t1"><code>NULL</code></td>
  </tr>
  <tr>
    <td class="t2"><code>-&gt;pw_home</code></td>
    <td class="t2"><code>NULL</code></td>
  </tr>
  <tr>
    <td class="t1"><code>-&gt;pw_shell</code></td>
    <td class="t1"><code>NULL</code></td>
  </tr>
  <tr>
    <td class="t2"><code>-&gt;pw_sgrp</code> (not fully implemented)</td>
    <td class="t2"><code>NULL</code></td>
  </tr>
  <tr>
    <td class="t1"><code>-&gt;sp_passwd</code></td>
    <td class="t1"><code>NULL</code></td>
  </tr>
  <tr>
    <td class="t2"><code>-&gt;sp_lastchg</code></td>
    <td class="t2"><code>0</code></td>
  </tr>
  <tr>
    <td class="t1"><code>-&gt;sp_min</code></td>
    <td class="t1"><code>VXDB_DFL_KEEPMIN</code></td>
  </tr>
  <tr>
    <td class="t2"><code>-&gt;sp_max</code></td>
    <td class="t2"><code>VXDB_DFL_KEEPMAX</code></td>
  </tr>
  <tr>
    <td class="t1"><code>-&gt;sp_warn</code></td>
    <td class="t1"><code>VXDB_DFL_WARNAGE</code></td>
  </tr>
  <tr>
    <td class="t2"><code>-&gt;sp_expire</code></td>
    <td class="t2"><code>VXDB_NO_EXPIRE</code></td>
  </tr>
  <tr>
    <td class="t1"><code>-&gt;sp_inact</code></td>
    <td class="t1"><code>VXDB_NO_INACTIVE</code></td>
  </tr>
  <tr>
    <td class="t2"><code>-&gt;vs_uuid</code></td>
    <td class="t2"><code>NULL</code></td>
  </tr>
  <tr>
    <td class="t1"><code>-&gt;vs_pvgrp</code></td>
    <td class="t1"><code>NULL</code></td>
  </tr>
  <tr>
    <td class="t2"><code>-&gt;vs_defer</code></td>
    <td class="t2"><code>0</code></td>
  </tr>
</table>

<p class="block">The internal buffer is not modified at all.</p>

<h2>vxdb_group_clean</h2>

<p class="block">Fills the user structure pointed to by user with the defaults,
which are:</p>

<table border="1">
  <tr>
    <td class="t1"><code>-&gt;gr_name</code></td>
    <td class="t1"><code>NULL</code></td>
  </tr>
  <tr>
    <td class="t2"><code>-&gt;gr_gid</code></td>
    <td class="t2"><code>VXDB_NOGID</code></td>
  </tr>
</table>

<p class="block">The internal buffer is not modified at all.</p>

<h2>vxdb_user_copy<br />
vxdb_group_copy</h2>

<h2>vxdb_user_dup<br />
vxdb_group_dup</h2>

<h2>vxdb_user_free<br />
vxdb_group_free</h2>

<p class="block">Free the structure or part thereof. If <code>heap</code> is
non-zero, the object is freed in its entirety (do not use it for stack
objects), or if it is zero, only free substructures which are on the heap.</p>

<h2>vxdb_user_nomodify<br />
vxdb_group_nomodify</h2>

<p class="block">Sets all fields in the <code>struct vxdb_user</code> or
<code>struct vxdb_group</code> so that when passed to
<code>vxdb_usermod()</code> or <code>vxdb_groupmod()</code>, respectively, as
a modify mask, no changes to the user or group would occurr.</p>

<h1>Per-user/per-group internal data</h1>

<p class="block">Some back-end modules store or cache the user data in memory
using our very own <code>struct vxdb_user</code> (and/or <code>struct
vxdb_group</code>, of course), where all pointers in these structs point to
the module's address space. (Address space as in "allocated by the module", not
in the sense of the operating system kernel.) Examples for this approach are
<i>libvxdb_shadow</i> and the demonstrational <i>libvxdb_nss</i>.</p>

<p class="block">Other modules, like <i>libvxdb_mysql</i> and
<i>libvxdb_ldap</i> do not use a cache, so there is only the LDAP/MySQL server
and the user program, but somewhere, the strings (such as username, home
directory, etc.) must be stored. Rather than having fixed-length character
arrays in <code>struct vxdb_user</code>, a dynamic buffer (exactly one) is
used, and the string members, e.g. <code>-&gt;pw_name</code> point to this
buffer, which will not cease to be valid once the module has been unloaded, and
therefore must be freed by the user application.<br />

Because of lack of ideas, the process of copying strings into a local buffer is
simply called "exporting" herein.</p>

<p class="code"><code><b>#</b>include &lt;vitalnix/libvxdb/libvxdb.h&gt;<br />
<br />
<b>void</b> vxdb_export_user(<b>const struct</b> vxdb_user <b>*</b>src, <b>struct</b> vxdb_user <b>*</b>dest);<br />
<b>void</b> vxdb_export_group(<b>const struct</b> vxdb_group <b>*</b>src, <b>struct</b> vxdb_group <b>*</b>dest);</code></p>

<h2>vxdb_export_user<br />
vxdb_export_group</h2>

<p class="block"><code>vxdb_export_user()</code> and
<code>vxdb_export_group()</code> will copy the strings pointed to by the
<code>struct vxdb_user</code> or <code>struct vxdb_group</code> members into
the local internal buffer, respectively. <code>src</code> and <code>dest</code>
might point to the same structure, but no pointer in <code>src</code> may point
to <code>dest</code>'s internal buffer.</p>

<?php include_once("Base-footer.php"); ?>
