<?php include_once("Base-header.php"); ?>

<h1>Name</h1>

<p class="block">vxdb_sgmapget - retrieve secondary groups of a user</p>

<h1>Synopsis</h1>

<p class="code"><code><b>#</b>include &lt;vitalnix/libvxdb/libvxdb.h&gt;<br />
<b>#</b>include &lt;vitalnix/libvxpdb/xafunc.h&gt;<br />
<br />
<b>int</b> vxdb_sgmapget(<b>struct vxdb_state *</b><i>handle</i>, <b>const char *</b><i>user</i>, <b>char ***</b><i>result</i>);</code></p>

<h1>Description</h1>

<p class="block"><code>vxdb_sgmapget()</code> will retrieve the list of
secondary (or "supplementary") groups the user is a member of. It will allocate
an appropriately-sized NULL-terminated array and store it in
<code>*list</code> <b>if</b> the user has one or more groups. If the user has
zero groups, or NULL is passed for <i>result</i>, nothing will be allocated.
Always check the return value of the function.</p>

<h1>Return value</h1>

<p class="block">On success, the number of groups is returned, which may be
zero or more. On error, a negative value is returned, indicating the error.</p>

<table border="1">
	<tr>
		<td><code><b>-ENOENT</b></code></td>
		<td>The user does not exist.</td>
	</tr>
	<tr>
		<td><code><b>-ENOMEM</b></code></td>
		<td>Out of memory.</td>
	</tr>
	<tr>
		<td><code><b>-EPERM</b></code></td>
		<td>Not enough privileges to complete this operation.</td>
	</tr>
</table>

<h1>Example</h1>

<p class="code"><code>
char **groups, **g;<br />
if (vxdb_sgmapget(dbh, "jengelh", &amp;groups) &gt; 0)<br />
&nbsp; &nbsp; for (g = groups; *g != NULL; ++g)<br />
&nbsp; &nbsp; &nbsp; &nbsp; printf("%s,", *g);</code></p>

<h1>See also</h1>

<p class="block"><a href="vxdb_sgmapadd.3.php">vxdb_sgmapadd(3)</a>,
<a href="vxdb_sgmapdel.3.php">vxdb_sgmapdel(3)</a></p>

<?php include_once("Base-footer.php"); ?>
