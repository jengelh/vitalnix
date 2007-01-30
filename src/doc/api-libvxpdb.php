<?php include_once("Base-header.php"); ?>

<h1>General</h1>

<p class="code"><tt><b>#</b>include &lt;vitanlix/libvxpdb/libvxpdb.h&gt;<br />
<br />
<b>struct</b> vxpdb_user;<br />
<b>struct</b> vxpdb_group;<br />
<b>struct</b> vxpdb_state;</p>

<h1>Function overview</h1>

<p class="code"><tt><b>#</b>include &lt;vitalnix/libvxpdb/libvxpdb.h&gt;<br />
<br />
<b>struct</b> vxpdb_state <b>*</b>vxpdb_load(<b>const char *</b>module);<br />
<b>void</b> vxpdb_unload(<b>struct</b> vxpdb_state <b>*</b>state);
</tt></p>

<h2>vxpdb_load</h2>

<p class="block">The <tt>vxpdb_load()</tt> function loads a PDB (user account
database, also known as password database) back-end driver, acquires and fixes
up all symbols and returns a struct describing the loaded module. The
<tt>module</tt> parameter can be one of:</p>

<ul>
  <li>the magic string <tt>"*"</tt> which will select the default module as
    listed in <tt>/etc/vitalnix/libvxpdb.conf</tt></li>
  <li>the filename of the module, e.g. <tt>"libvxdb_shadow.so"</tt></li>
  <li>the canonical name of the module, e.g. <tt>"mysql"</tt> (which is
    completed to <tt>"libvxdb_mysql.so"</tt>)</li>
</ul>

<p class="block">The second way is not widely used, because it the filename
extension of the shared library depends on the architecture Vitalnix runs on.
Although the third point only lists <tt>.so</tt> as an example, the
<tt>.dll</tt> extension is also tried when completing to a filename, even under
non-Win32. Note that for any module which is not named in the fashion of
<tt>libvxdb_*.so</tt>, there is only the second way.</p>

<p class="block">On failure, <tt>vxpdb_load()</tt> will return <tt>NULL</tt>
and <tt>errno</tt> will be set.</p>

<h2>vxpdb_unload</h2>

<p class="block">Unloads the PDB module.</p>

<h1>Function overview&nbsp;-- Module access functions</h1>

<p class="code"><tt><b>#</b>include &lt;vitalnix/libvxpdb/xafunc.h&gt;<br />
<br />
<b>int</b> vxpdb_open(<b>struct</b> vxpdb_state <b>*</b>state, <b>long</b> flags);<br />
<b>void</b> vxpdb_close(<b>struct</b> vxpdb_state <b>*</b>state);<br />
<b>int</b> vxpdb_modctl(<b>struct</b> vxpdb_state <b>*</b>state, <b>long</b> command, <b>...</b>);<br />
<b>int</b> vxpdb_lock(<b>struct</b> vxpdb_state <b>*</b>state);<br />
<b>int</b> vxpdb_unlock(<b>struct</b> vxpdb_state <b>*</b>state);<br />
<b>int</b> vxpdb_useradd(<b>struct</b> vxpdb_state <b>*</b>state, <b>const struct</b> vxpdb_user <b>*</b>user);<br />
<b>int</b> vxpdb_usermod(<b>struct</b> vxpdb_state <b>*</b>state, <b>const struct</b> vxpdb_user <b>*</b>search_mask, <b>const struct</b> vxpdb_user <b>*</b>replace_mask);<br />
<b>int</b> vxpdb_userdel(<b>struct</b> vxpdb_state <b>*</b>state, <b>const struct</b> vxpdb_user <b>*</b>search_mask);<br />
<b>int</b> vxpdb_userinfo(<b>struct</b> vxpdb_state <b>*</b>state, <b>const struct</b> vxpdb_user <b>*</b>search_mask, <b>struct</b> vxpdb_user <b>*</b>result, <b>size_t</b> size);<br />
<b>void *</b>vxpdb_usertrav_init(<b>struct</b> vxpdb_state <b>*</b>state);<br />
<b>int</b> vxpdb_usertrav_walk(<b>struct</b> vxpdb_state <b>*</b>state, <b>void *</b>data, <b>struct</b> vxpdb_user <b>*</b>result);<br />
<b>void</b> vxpdb_usertrav_free(<b>struct</b> vxpdb_state <b>*</b>state, <b>void *</b>data);<br />
<b>int</b> vxpdb_groupadd(<b>struct</b> vxpdb_state <b>*</b>state, <b>const struct</b> vxpdb_group <b>*</b>group);<br />
<b>int</b> vxpdb_groupmod(<b>struct</b> vxpdb_state <b>*</b>state, <b>const struct</b> vxpdb_group <b>*</b>search_mask, <b>const struct</b> vxpdb_group <b>*</b>replace_mask);<br />
<b>int</b> vxpdb_groupdel(<b>struct</b> vxpdb_state <b>*</b>state, <b>const struct</b> vxpdb_group <b>*</b>search_mask);<br />
<b>int</b> vxpdb_groupinfo(<b>struct</b> vxpdb_state <b>*</b>state, <b>const struct</b> vxpdb_group <b>*</b>search_mask, <b>struct</b> vxpdb_group <b>*</b>result, <b>size_t</b> size);<br />
<b>void *</b>vxpdb_grouptrav_init(<b>struct</b> vxpdb_state <b>*</b>state);<br />
<b>int</b> vxpdb_grouptrav_walk(<b>struct</b> vxpdb_state <b>*</b>state, <b>void *</b>data, <b>struct</b> vxpdb_group <b>*</b>result);<br />
<b>void</b> vxpdb_grouptrav_free(<b>struct</b> vxpdb_state <b>*</b>state, <b>void *</b>data);
</tt></p>

<h2>vxpdb_open</h2>

<p class="block">Open the password database associated with state using
<tt>flags</tt>. <tt>state</tt> must have been obtained using
<tt>vxpdb_load()</tt> before.</p>

<h2>vxpdb_close</h2>

<h2>vxpdb_lock</h2>

<p class="block">Lock the database. Returns an AEE code. If the database does
not implement it, either zero or <tt>-ENOSYS</tt> is returned.</p>

<h2>vxpdb_unlock</h2>

<p class="block">Unlock the database. Always succeeds (it really depends on the
PDB module&nbsp;-- and returns an AEE code).</p>

<h2>vxpdb_useradd</h2>

<p class="block">Returns <tt>-EINVAL</tt> if the username is empty, or
<tt>-EEXIST</tt> if the username already exists. It is implementation-defined
whether the creation of accounts with non-unique UID is allowed. If forbidden,
<tt>-EEXIST</tt> is returned too.</p>

<h2>vxpdb_usermod</h2>

<p class="block">Returns <tt>-EINVAL</tt> if both username and uid in the
search mask are empty, or returns <tt>-ENOENT</tt> if the user was not
found.</p>

<h2>vxpdb_userdel</h2>

<p class="block">Returns <tt>-EINVAL</tt> if both username and uid in the
search mask are empty, or returns <tt>-ENOENT</tt> if the user was not
found.</p>

<h1>Function overview&nbsp;-- Extra access functions</h1>

<p class="code"><tt><b>#</b>include &lt;vitalnix/libvxpdb/xwfunc.h&gt;<br />
<br />
<b>int</b> vxpdb_getpwnam(<b>struct</b> vxpdb_state <b>*</b>state, <b>const char *</b>user, <b>struct</b> vxpdb_user <b>*</b>result);<br />
<b>int</b> vxpdb_getpwuid(<b>struct</b> vxpdb_state <b>*</b>state, <b>long</b> uid, <b>struct</b> vxpdb_user <b>*</b>result);<br />
<b>int</b> vxpdb_getgrnam(<b>struct</b> vxpdb_state <b>*</b>state, <b>const char *</b>group, <b>struct</b> vxpdb_group <b>*</b>result);<br />
<b>int</b> vxpdb_getgrgid(<b>struct</b> vxpdb_state <b>*</b>state, <b>long</b> gid, <b>struct</b> vxpdb_group <b>*</b>result);
</tt></p>

<p class="block">These functions provide simple wrappers around
<tt>vxpdb_userinfo()</tt> and <tt>vxpdb_groupinfo()</tt> that take the burden
off the user to create the search mask beforehand if one only wants to search
for one paramter (in this case either name or uid/gid). Just as their
underlying function, they will all allocate the internal data buffer in the
<tt>struct vxpdb_user</tt>/<tt>vxpdb_group</tt> if necessary.</p>

<h1>Auxiliary functions</h1>

<p class="code"><tt><b>#</b>include &lt;vitalnix/libvxpdb/libvxpdb.h&gt;<br />
<br />
<b>void</b> vxpdb_user_clean(<b>struct</b> vxpdb_user <b>*</b>user);<br />
<b>void</b> vxpdb_user_copy(<b>struct</b> vxpdb_user <b>*</b>dest, <b>const struct</b> vxpdb_user <b>*</b>src);<br />
<b>struct</b> vxpdb_user <b>*</b>vxpdb_user_dup(<b>const struct</b> vxpdb_user <b>*</b>src);<br />
<b>void</b> vxpdb_user_free(<b>struct</b> vxpdb_user <b>*</b>user, <b>int</b> heap);<br />
<b>int</b> vxpdb_user_match(<b>const struct</b> vxpdb_user <b>*</b>user, <b>const struct</b> vxpdb_user <b>*</b>mask);<br />
<b>void</b> vxpdb_user_nomodify(<b>struct</b> vxpdb_user <b>*</b>user);<br />
<br />
<b>void</b> vxpdb_group_clean(<b>struct</b> vxpdb_group <b>*</b>group);<br />
<b>void</b> vxpdb_group_copy(<b>struct</b> vxpdb_group <b>*</b>dest, <b>const struct</b> vxpdb_group <b>*</b>src);<br />
<b>struct</b> vxpdb_user <b>*</b>vxpdb_group_dup(<b>const struct</b> vxpdb_group <b>*</b>src);<br />
<b>void</b> vxpdb_group_free(<b>struct</b> vxpdb_user <b>*</b>user, <b>int</b> heap);<br />
<b>int</b> vxpdb_group_match(<b>const struct</b> vxpdb_group <b>*</b>group, <b>const struct</b> vxpdb_group <b>*</b>mask);<br />
<b>void</b> vxpdb_group_nomodify(<b>struct</b> vxpdb_group <b>*</b>group);</tt></p>

<h2>vxpdb_user_clean</h2>

<p class="block">Fills the user structure pointed to by <tt>user</tt> with the
defaults, which are:</p>

<table border="1" class="bordered">
  <tr>
    <td class="t1"><tt>-&gt;pw_name</tt></td>
    <td class="t1"><tt>NULL</tt></td>
  </tr>
  <tr>
    <td class="t2"><tt>-&gt;pw_uid</tt></td>
    <td class="t2"><tt>PDB_NOUID</tt></td>
  </tr>
  <tr>
    <td class="t1"><tt>-&gt;pw_gid</tt></td>
    <td class="t1"><tt>PDB_NOGID</tt></td>
  </tr>
  <tr>
    <td class="t2"><tt>-&gt;pw_igrp</tt> (NOT IMPLEMENTED YET)</td>
    <td class="t2"><tt>NULL</tt></td>
  </tr>
  <tr>
    <td class="t1"><tt>-&gt;pw_real</tt></td>
    <td class="t1"><tt>NULL</tt></td>
  </tr>
  <tr>
    <td class="t2"><tt>-&gt;pw_home</tt></td>
    <td class="t2"><tt>NULL</tt></td>
  </tr>
  <tr>
    <td class="t1"><tt>-&gt;pw_shell</tt></td>
    <td class="t1"><tt>NULL</tt></td>
  </tr>
  <tr>
    <td class="t2"><tt>-&gt;pw_sgrp</tt> (not fully implemented)</td>
    <td class="t2"><tt>NULL</tt></td>
  </tr>
  <tr>
    <td class="t1"><tt>-&gt;sp_passwd</tt></td>
    <td class="t1"><tt>NULL</tt></td>
  </tr>
  <tr>
    <td class="t2"><tt>-&gt;sp_lastchg</tt></td>
    <td class="t2"><tt>0</tt></td>
  </tr>
  <tr>
    <td class="t1"><tt>-&gt;sp_min</tt></td>
    <td class="t1"><tt>PDB_DFL_KEEPMIN</tt></td>
  </tr>
  <tr>
    <td class="t2"><tt>-&gt;sp_max</tt></td>
    <td class="t2"><tt>PDB_DFL_KEEPMAX</tt></td>
  </tr>
  <tr>
    <td class="t1"><tt>-&gt;sp_warn</tt></td>
    <td class="t1"><tt>PDB_DFL_WARNAGE</tt></td>
  </tr>
  <tr>
    <td class="t2"><tt>-&gt;sp_expire</tt></td>
    <td class="t2"><tt>PDB_NO_EXPIRE</tt></td>
  </tr>
  <tr>
    <td class="t1"><tt>-&gt;sp_inact</tt></td>
    <td class="t1"><tt>PDB_NO_INACTIVE</tt></td>
  </tr>
  <tr>
    <td class="t2"><tt>-&gt;vs_uuid</tt></td>
    <td class="t2"><tt>NULL</tt></td>
  </tr>
  <tr>
    <td class="t1"><tt>-&gt;vs_pvgrp</tt></td>
    <td class="t1"><tt>NULL</tt></td>
  </tr>
  <tr>
    <td class="t2"><tt>-&gt;vs_defer</tt></td>
    <td class="t2"><tt>0</tt></td>
  </tr>
</table>

<p class="block">The internal buffer is not modified at all.</p>

<h2>vxpdb_group_clean</h2>

<p class="block">Fills the user structure pointed to by user with the defaults,
which are:</p>

<table border="1" class="bordered">
  <tr>
    <td class="t1"><tt>-&gt;gr_name</tt></td>
    <td class="t1"><tt>NULL</tt></td>
  </tr>
  <tr>
    <td class="t2"><tt>-&gt;gr_gid</tt></td>
    <td class="t2"><tt>PDB_NOGID</tt></td>
  </tr>
</table>

<p class="block">The internal buffer is not modified at all.</p>

<h2>vxpdb_user_copy<br />
vxpdb_group_copy</h2>

<h2>vxpdb_user_dup<br />
vxpdb_group_dup</h2>

<h2>vxpdb_user_free<br />
vxpdb_group_free</h2>

<p class="block">Free the structure or part thereof. If <tt>heap</tt> is
non-zero, the object is freed in its entirety (do not use it for stack
objects), or if it is zero, only free substructures which are on the heap.</p>

<h2>vxpdb_user_match<br />
vxpdb_group_match</h2>

<p class="block">Returns non-zero if the user or group can be matched with
mask, respectively. An empty mask (such as produced by
<tt>vxpdb_user_clean</tt>) will always match.</p>

<h2>vxpdb_user_nomodify<br />
vxpdb_group_nomodify</h2>

<p class="block">Sets all fields in the <tt>struct vxpdb_user</tt> or
<tt>struct vxpdb_group</tt> so that when passed to <tt>vxpdb_usermod()</tt> or
<tt>vxpdb_groupmod()</tt>, respectively, as a modify mask, no changes to the
user or gruop would occurr.</p>

<h1>Per-user/per-group internal data</h1>

<p class="block">Some back-end modules store or cache the user data in memory
using our very own <tt>struct vxpdb_user</tt> (and/or <tt>struct
vxpdb_group</tt>, of course), where all pointers in these structs point to the
module's address space. (Address space as in "allocated by the module", not in
the sense of the operating system kernel.) Examples for this approach are
<i>libvxdb_shadow</i> and the demonstrational <i>libvxdb_nss</i>.</p>

<p class="block">Other modules, like <i>libvxdb_mysql</i> and
<i>libvxdb_ldap</i> do not use a cache, so there is only the LDAP/MySQL server
and the user program, but somewhere, the strings (such as username, home
directory, etc.) must be stored. Rather than having fixed-length character
arrays in <tt>struct vxpdb_user</tt>, a dynamic buffer (exactly one) is used,
and the string members, e.g. <tt>-&gt;pw_name</tt> point to this buffer, which
will not cease to be valid once the module has been unloaded, and therefore
must be freed by the user application.<br />

Because of lack of ideas, the process of copying strings into a local buffer is
simply called "exporting" herein.</p>

<p class="code"><tt><b>#</b>include &lt;vitalnix/libvxpdb/libvxpdb.h&gt;<br />
<br />
<b>void</b> vxpdb_export_user(<b>const struct</b> vxpdb_user <b>*</b>src, <b>struct</b> vxpdb_user <b>*</b>dest);<br />
<b>void</b> vxpdb_export_group(<b>const struct</b> vxpdb_group <b>*</b>src, <b>struct</b> vxpdb_group <b>*</b>dest);<br />
<b>void *</b>VX_NDALLOC(<b>struct</b> vxpdb_user <b>*</b>user, <b>size_t</b> size);<br />
<b>void *</b>VX_NDALLOC(<b>struct</b> vxpdb_group <b>*</b>group, <b>size_t</b> size);<br />
<b>void</b> VX_NDFREE(<b>struct</b> vxpdb_user <b>*</b>user);<br />
<b>void</b> VX_NDFREE(<b>struct</b> vxpdb_group <b>*</b>group);</tt></p>

<h2>vxpdb_export_user<br />
vxpdb_export_group</h2>

<p class="block"><tt>vxpdb_export_user()</tt> and <tt>vxpdb_export_group()</tt>
will copy the strings pointed to by the <tt>struct vxpdb_user</tt> or
<tt>struct vxpdb_group</tt> members into the local internal buffer,
respectively. <tt>src</tt> and <tt>dest</tt> might point to the same structure,
but no pointer in <tt>src</tt> may point to <tt>dest</tt>'s internal
buffer.</p>

<h2>VX_NDALLOC</h2>

<p class="block">This macro resizes the internal data buffer for this
user/group to at least <tt>size</tt> bytes, if it is smaller than <tt>size</tt>
bytes. It is normally only called from within the back-end modules as a result
of exporting.</p>

<h2>VX_NDFREE</h2>

<p class="block">Frees the internal buffer if there is one allocated. This
should be done when the structure is about to go out of scope. You do not need
to free it if you can make sure the internal buffer was not allocated. See the
section "Module access functions" to find out which functions do allocate the
internal buffer.</p>

<?php include_once("Base-footer.php"); ?>
