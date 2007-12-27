<?php include_once("Base-header.php"); ?>

<h1>Name</h1>

<p>vxtryauth&nbsp;-- authenticate with PAM</p>

<h1>Syntax</h1>

<p class="code"><code><b>vxtryauth</b> [<b>-v</b>]</code></p>

<h1>Description</h1>

<p class="block"><i>vxtryauth</i> is a helper utility for programs and/or
scripts which do not have intermediate access to the libvxdb C API, but still
want to authenticate a user. <i>vxtryauth</i> accepts the username and password
on standard input and the exit status contains the result of the
authentication.</p>

<h1>Options</h1>

<table border="1">
	<tr>
		<td class="t1n"><code><b>-v</b></code></td>
		<td class="t1">Print error messages, should an error
		happen.</td>
	</tr>
</table>

<h1>Exit status</h1>

<p class="block"><code>0</code> is returned on success, <code>127</code> if
there was a problem during option parsing, or any other value indicating the
PAM error code. If the <code>-v</code> option is specified, additional error
messages&nbsp;-- if any&nbsp;-- will be printed to the standard error file
descriptor (stderr).</p>

<?php include_once("Base-footer.php"); ?>
