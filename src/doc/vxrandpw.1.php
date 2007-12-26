<?php include_once("Base-header.php"); ?>

<h1>Name</h1>

<p>vxrandpw&nbsp;-- generate random password</p>

<h1>Syntax</h1>

<p class="code"><code><b>vxrandpw</b> [<b>-01BCDJMZcr</b>] [<b>-l</b>
<i>length</i>]</code></p>

<h1>Description</h1>

<p class="block"><i>vxrandpw</i> is an interface to the Vitalnix password
generator. The password is output once in plain and once in encrypted form.
Various flags control the generation and encryption behavior.</p>

<h1>Options</h1>

<h2>Generation</h2>

<table border="1">
	<tr>
		<td class="t1n"><code><b>-0</b></code></td>
		<td class="t1">Possibly have a digit in the password.</td>
	</tr>
	<tr>
		<td class="t2n"><code><b>-1</b></code></td>
		<td class="t2">Always have a digit in the password.</td>
	</tr>
	<tr>
		<td class="t1n"><code><b>-C</b></code></td>
		<td class="t1">Always have an uppercase character in the
		password.</td>
	</tr>
	<tr>
		<td class="t2n"><code><b>-J</b></code></td>
		<td class="t2">Use "GENPW_JP" generation method.</td>
	</tr>
	<tr>
		<td class="t1n"><code><b>-Z</b></code></td>
		<td class="t1">Use "GENPW_ZH" generation method.</td>
	</tr>
	<tr>
		<td class="t2n"><code><b>-c</b></code></td>
		<td class="t2">Possibly have an uppercase character in the
		password.</td>
	</tr>
	<tr>
		<td class="t1n"><code><b>-l</b></code> <i>length</i></td>
		<td class="t1">Password length</td>
	</tr>
	<tr>
		<td class="t2n"><code><b>-r</b></code></td>
		<td class="t2">Use "random" generation method.</td>
	</tr>
</table>

<h2>Encryption</h2>

<table border="1">
	<tr>
		<td class="t1n"><code><b>-B</b></code></td>
		<td class="t1">Use Blowfish encryption</td>
	</tr>
	<tr>
		<td class="t2n"><code><b>-D</b></code></td>
		<td class="t2">Use DES encryption (this might not be available
		on all platforms)</td>
	</tr>
	<tr>
		<td class="t1n"><code><b>-M</b></code></td>
		<td class="t1">Use MD5 encryption (this might not be available
		on all platforms)</td>
	</tr>
</table>

<?php include_once("Base-footer.php"); ?>
