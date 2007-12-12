<?php include_once("Base-header.php"); ?>

<h1>Name</h1>

<p class="block">vxdb_sgmapadd - add a secondary group mapping</p>

<h1>Synopsis</h1>

<p class="code"><code><b>#</b>include &lt;vitalnix/libvxdb/libvxdb.h&gt;<br />
<b>#</b>include &lt;vitalnix/libvxdb/xafunc.h&gt;<br />
<br />
<b>int</b> vxdb_sgmapadd(<b>struct vxdb_state *</b><i>handle</i>, <b>const char *</b><i>user</i>, <b>const char *</b><i>group</i>);</code></p>

<h1>Description</h1>

<p class="block">The <code>vxdb_sgmapadd()</code> function is used to add a new
secondary group mapping, i.e. it will add the group to the user's list of
secondary (or "supplementary") groups.</p>

<h1>Return value</h1>

<p class="block">On success, positive non-zero is returned. On error, zero or a
negative value is returned, indicating the error.</p>

<table border="1">
	<tr>
		<td><code>-ENOENT</code></td>
		<td>The user or the group does not exist.</td>
	</tr>
	<tr>
		<td><code>-EEXIST</code></td>
		<td>The mapping already exists.</td>
	</tr>
	<tr>
		<td><code>-EPERM</code></td>
		<td>Not enough privileges to complete this operation.</td>
	</tr>
</table>

<h1>See also</h1>

<p class="block"><a href="vxdb_sgmapget.3.php">vxdb_sgmapget(3)</a>,
<a href="vxdb_sgmapdel.3.php">vxdb_sgmapdel(3)</a></p>

<?php include_once("Base-footer.php"); ?>
