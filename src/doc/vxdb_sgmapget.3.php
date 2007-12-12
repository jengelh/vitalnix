<?php include_once("Base-header.php"); ?>

<h1>Name</h1>

<p class="block">vxdb_sgmapget - retrieve secondary groups of a user</p>

<h1>Synopsis</h1>

<p class="code"><code><b>#</b>include &lt;vitalnix/libvxpdb/libvxpdb.h&gt;<br />
<b>#</b>include &lt;vitalnix/libvxpdb/xafunc.h&gt;<br />
<br />
<b>int</b> vxdb_sgmapget(<b>struct vxdb_state *</b><i>handle</i>, <b>const char *</b><i>user</i>, <b>char ***</b><i>result</i>);</code></p>

<h1>Description</h1>

<p class="block"><code>vxdb_sgmapget()</code> will retrieve the list of
secondary (or "supplementary") groups the user is a member of. It will allocate
an appropriately-sized NULL-terminated array and store it in
<code>*list</code>.</p>

<h1>Return value</h1>

<p class="block">On success, the number of groups is returned, which may be
zero or more. On error, a negative value is returned, indicating the error.</p>

<h1>Example</h1>

<p class="code"><code>
<b>char **</b>groups, <b>**</b>g;<br />
vxdb_sgmapget(dbh, "jengelh", <b>&amp;</b>groups);<br />
<b>for</b> (g <b>=</b> groups; <b>*</b>g <b>!=</b> NULL; ++g)<br />
 &nbsp; &nbsp; printf("%s,", <b>*</b>g);</code></p>

<h1>See also</h1>

<p class="block"><a href="vxdb_sgmapadd.3.php">vxdb_sgmapadd(3)</a>,
<a href="vxdb_sgmapdel.3.php">vxdb_sgmapdel(3)</a></p>

<?php include_once("Base-footer.php"); ?>
