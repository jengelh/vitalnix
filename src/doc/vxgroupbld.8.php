<?php include_once("Base-header.php"); ?>

<h1>Usage</h1>

<p class="code"><code><b>groupbld</b> [<b>-qv</b>] [<b>-G</b> <i>group</i>]
[<b>-M</b> <i>module</i>] [<b>-p</b> <i>prefix</i>]</code></p>

<h1>Description</h1>

<p class="block">vxgroupbld updates the system (UNIX) group membership of all
users that have a Vitalnix group tag, and moves them from their current
Vitalnix-UNIX-Group (VUG) to the new VUG that matches the Vitalnix group tag.
vxgroupbld will create the new VUG if necessary. The VUGs can then be used for
filesystem permissions, for example.</p>

<p class="block">VUGs are generally prefixed with a fixed string so as to
distinguish them from regular UNIX groups. The default prefix is
<code>vg-</code>, which allows the Vitalnix group to start with a number. For
example, if it was <code>10NK</code>, the VUG would be called
<code>vg-10NK</code>. <b>NOTE: Using a potentially ambiguous or no prefix will
obviously cause users to lose membership in secondary groups</b>, because
vxgroupbld operates on all secondary groups that have this prefix.</p>

<p class="block">vxgroupbld is typically used after updating the
Vitalnix group tag; running it after vxusersync is a common case, e.g.
when students move up one grade.</p>

<h1>Options</h1>

<table border="1">
	<tr>
		<td class="t1n"><code><b>-G</b> <i>group</i></code></td>
		<td class="t1">Only operate on users which have <i>group</i> as
		their primary group. This can either be a group's name or its
		GID.</td>
	<tr>
		<td class="t2n"><code><b>-M</b> <i>database</i></code></td>
		<td class="t2">Uses the specified database rather than the
		default one defined in the VXPDB configuration file.</td>
	</tr>
	<tr>
		<td class="t1n"><code><b>-p</b> <i>prefix</i></code></td>
		<td class="t1">Prefix to use for VUGs. The default is
		<code>vg-</code>.</td>
	</tr>
	<tr>
		<td class="t2n"><code><b>-q</b></code></td>
		<td class="t2">Quiet operation. This is the default.</td>
	</tr>
	<tr>
		<td class="t1n"><code><b>-v</b></code></td>
		<td class="t1">Verbose operation. This will print progress
		information messages every second.</td>
	</tr>
</table>

<h1>Exit status</h1>

<p class="block">vxgroupbld will return 0 if all operations completed
successfully. On error, it will abort right away and returns non-zero.</p>

<?php include_once("Base-footer.php"); ?>
