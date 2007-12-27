<?php include_once("Base-header.php"); ?>

<h1>Description</h1>

<p class="block">The <i>dummy</i> driver does exactly what it name suggests.
It returns success where feasible, otherwise failure. It is internal to
<i>libvxdb</i> and will be used as a fallback whenever a specific database
driver does not provide its own variant of the function.</p>

<table border="1">
	<tr>
		<td class="t1"><code>-&gt;init()<br />
			-&gt;open()</code></td>
		<td class="t1"><code>1</code> (success)</td>
	</tr>
	<tr>
		<td class="t2"><code>-&gt;close()<br />
			-&gt;exit()</code></td>
		<td class="t2">void</td>
	</tr>
	<tr>
		<td class="t1"><code>-&gt;modctl(</code>VXDB_COUNT_USERS<code>)<br />
			-&gt;modctl(</code>VXDB_COUNT_GROUPS<code>)</td>
		<td class="t1"><code>0</code> (no users/groups)</td>
	</tr>
	<tr>
		<td class="t2"><code>-&gt;modctl(</code>*<code>)</code></td>
		<td class="t2"><code>-ENOSYS</code> ("Function not
		implemented")</td>
	</tr>
	<tr>
		<td class="t1"><code>-&gt;lock()<br />
			-&gt;unlock()</code></td>
		<td class="t1"><code>1</code> (success)</td>
	</tr>
	<tr>
		<td class="t2"><code>-&gt;useradd()<br />
			-&gt;usermod()<br />
			-&gt;userdel()<br />
			-&gt;groupadd()<br />
			-&gt;groupmod()<br />
			-&gt;groupdel()</code></td>
		<td class="t2"><code>-EPERM</code> ("Operation not
		permitted")</td>
	</tr>
	<tr>
		<td class="t1"><code>-&gt;usertrav_init()<br />
			-&gt;grouptrav_init()</code></td>
		<td class="t1">non-<code>NULL</code> (traverser init
		succeeded)</td>
	</tr>
	<tr>
		<td class="t2"><code>-&gt;usertrav_walk()<br />
			-&gt;grouptrav_walk()</code></td>
		<td class="t2"><code>0</code> (no more users/groups)</td>
	</tr>
	<tr>
		<td class="t1"><code>-&gt;usertrav_free()<br />
			-&gt;grouptrav_free()</code></td>
		<td class="t1">void</td>
	</tr>
</table>

<?php include_once("Base-footer.php"); ?>