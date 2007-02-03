<?php include_once("Base-header.php"); ?>

<h1>Description</h1>

<p class="block"><i>libvxmdsync</i> contains the code for synchronizing the EDS
user list to the system user database.</p>

<h1>Structures</h1>

<p class="code"><code>
<b>#</b>include &lt;libvxmdsync/libvxmdsync.h&gt;<br />
<br />
<b>struct</b> mdsync_config {<br />
&nbsp; &nbsp; <b>struct</b> vxpdb_user user_defaults;<br />
&nbsp; &nbsp; <b>int</b> new_pw_length, genpw_type, crypw_type;<br />
&nbsp; &nbsp; <b>int</b> db_force_flush;<br />
&nbsp; &nbsp; <b>long</b> home_umask;<br />
&nbsp; &nbsp; <b>char *</b>skeleton_dir;<br />
&nbsp; &nbsp; <b>char *</b>master_preadd, <b>*</b>master_postadd,<br />
&nbsp; &nbsp; &nbsp; &nbsp; &nbsp;<b>*</b>master_premod, <b>*</b>master_postmod,<br />
&nbsp; &nbsp; &nbsp; &nbsp; &nbsp;<b>*</b>master_predel, <b>*</b>master_postdel,<br />
&nbsp; &nbsp; &nbsp; &nbsp; &nbsp;<b>*</b>user_preadd, <b>*</b>user_postadd,<br />
&nbsp; &nbsp; &nbsp; &nbsp; &nbsp;<b>*</b>user_premod, <b>*</b>user_postmod,<br />
&nbsp; &nbsp; &nbsp; &nbsp; &nbsp;<b>*</b>user_predel, <b>*</b>user_postdel;<br />
&nbsp; &nbsp; <b>char *</b>home_base;<br />
&nbsp; &nbsp; <b>long</b> split_level;<br />
};<br />
<br />
<b>struct</b> mdsync_workspace {<br />
&nbsp; &nbsp; <b>struct</b> mdsync_config config;<br />
&nbsp; &nbsp; <b>struct</b> vxpdb_state <b>*</b>database;<br />
&nbsp; &nbsp; ...<br />
};
</code></p>

<h1>Function overview</h1>

<p class="code"><code>
<b>#</b>include &lt;libvxmdsync/libvxmdsync.h&gt;<br />
<br />
<b>struct</b> mdsync_workspace <b>*</b>mdsync_init(<b>void</b>);<br />
<b>int</b> mdsync_prepare_group(<b>struct</b> mdsync_workspace <b>*</b>ws, <b>const char *</b>group);<br />
<b>int</b> mdsync_read_file(<b>struct</b> mdsync_workspace <b>*</b>ws, <b>const char *</b>filename, <b>const char *</b>filetype);<br />
<b>int</b> mdsync_open_log(<b>struct</b> mdsync_workspace <b>*</b>ws, <b>const char *</b>filename);<br />
<b>int</b> mdsync_compare(<b>struct</b> mdsync_workspace <b>*</b>ws);<br />
<b>int</b> mdsync_compare_simple(<b>struct</b> mdsync_workspace <b>*</b>ws);<br />
<b>int</b> mdsync_fixup(<b>struct</b> mdsync_workspace <b>*</b>ws);<br />
<b>int</b> mdsync_add(<b>struct</b> mdsync_workspace <b>*</b>ws);<br />
<b>int</b> mdsync_upd(<b>struct</b> mdsync_workspace <b>*</b>ws);<br />
<b>int</b> mdsync_del(<b>struct</b> mdsync_workspace <b>*</b>ws);<br />
<b>void</b> mdsync_free(<b>struct</b> mdsync_workspace <b>*</b>ws);
</code></p>

<h2>mdsync_compare</h2>

<p class="block">Compares <code>ws-&gt;add_req</code> to the system user
database.</p> Users which are both in <code>ws-&gt;add_req</code> and the PDB
are moved from <code>ws-&gt;add_req</code> to <code>ws-&gt;keep_req</code>.</p>

<p class="block">If deferred deletion is not configured, puts users, which have
been removed from the EDS, but still exist in the PDB, into
<code>ws-&gt;delete_now</code>.</p>

<p class="block">If deferred deletion is active, puts users, which do not exist
on the EDS anymore, into <code>ws-&gt;defer_*</code> depending on their
deferred deletion status.</p>

<p class="block">Puts all currently used usernames into
<code>ws-&gt;lnlist</code>, including those that will be deleted, so usernames
do not get reused right away.</p>

<h2>mdsync_compare_simple</h2>

<p class="bock">Puts all usernames from the PDB into
<code>ws-&gt;lnlist</code>. This is used for single-user adds, where deletion
lists are not needed at all.</p>

<h2>mdsync_fixup</h2>

<p class="block">Weeds out any double usernames in <code>ws-&gt;add_req</code>
for new users by appending an appropriate index number to their username,
necessarily truncating the name. For example, if there already was a user with
the login name <code>jengelh</code>, another one would get
<code>jengelh1</code>, the next <code>jengelh2</code>, etc. The maximum
characters for a username is hardcoded in <i>libvxmdsync</i> to eight chars (do
not change it), making the truncation pattern look like this in the presence of
excess users: <code>jengel10, jenge100, jeng1000</code>.</p>

<h2>mdsync_init</h2>

<h1>Order</h1>

<p class="block">The synchronization process is divided into a number of
functions to facilitate error processing and arguments needed to pass in at
once (the <code>struct mdsync_workspace</code> is already big). The exact order
of calls to be made is:</p>

<ol>
  <li><code>mdsync_init()</code></li>
  <li><code>mdsync_prepare_group()</code></li>
  <li><code>mdsync_read_file()</code></li>
  <li><code>mdsync_open_log()</code></li>
  <li>Exactly one of:</li>
    <ul>
      <li><code>mdsync_compare()</code></li>
      <li><code>mdsync_compare_simple()</code></li>
    </ul></li>
  <li><code>mdsync_fixup()</code></li>
  <li>One or more of the following:<br />
    <ul>
      <li><code>mdsync_add()</code></li>
      <li><code>mdsync_upd()</code></li>
      <li><code>mdsync_del()</code></li>
    </ul></li>
  <li><code>mdsync_free()</code></li>
</ol>

<?php include_once("Base-footer.php"); ?>
