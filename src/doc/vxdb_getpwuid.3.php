<?php include_once("Base-header.php"); ?>

<h1>Name</h1>

<p class="block">vxdb_getpwuid - get user information by UID</p>

<h1>Synopsis</h1>

<p class="code"><code><b>#</b>include &lt;vitalnix/libvxdb/libvxdb.h&gt;<br />
<b>#</b>include &lt;vitalnix/libvxdb/xafunc.h&gt;<br />
<br />
<b>int</b> vxdb_getpwnam(<b>struct vxdb_state *</b><i>handle</i>, <b>const char *</b><i>username</i>, <b>struct vxdb_user *</b><i>result</i>);<br />
<b>int</b> vxdb_getpwuid(<b>struct vxdb_state *</b><i>handle</i>, <b>unsigned int</b> <i>uid</i>, <b>struct vxdb_user *</b><i>result</i>);<br />
<br />
Link with <i>-lvxdb</i>.</code></p>

<h1>Description</h1>

<p class="block">Retrieves general information, such as login name, numeric
UID, primary group, home directory, default shell, etc. about the user given by
<i>username</i> or <i>uid</i>. <i>result</i> should point to a valid
<code>struct vxdb_state</code>, i.e. it must have been zeroed or been filled
before. See the example below.</p>

<h1>Return value</h1>

<p class="block">On success, positive non-zero is returned. If the user was not
found, zero is returned. On error, a negative value is returned, indicating the
error.</p>

<p class="block">No typical errors specified.</p>

<h1>Example</h1>

<p class="code"><code>
struct vxdb_user user = {};<br />
int ret;<br />
if ((ret = vxdb_getpwnam(dbh, "jengelh", &amp;user)) &lt; 0)<br />
&nbsp; &nbsp; perror("vxdb_getpwnam");<br />
else if (ret == 0)<br />
&nbsp; &nbsp; fprintf(stderr, "The user does not exist\n");<br />
else<br />
&nbsp; &nbsp; do_something();
</code></p>

<p class="block">Alternatively, you may use</p>

<p class="code"><code>
struct vxdb_state user;<br />
memset(&amp;user, 0, sizeof(*user));
</code></p>

<h1>See also</h1>

<p><a href="vitalnix.7.php">vitalnix</a>(7)</p>

<?php include_once("Base-footer.php"); ?>
