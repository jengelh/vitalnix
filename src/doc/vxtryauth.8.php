<?php include_once("Base-header.php"); ?>

<h1>Usage</h1>

<p class="code"><code>tryauth <b>[</b>-v<b>]</b></code></p>

<h1>Description</h1>

<p class="block"><i>tryauth</i> is a helper utility for programs and/or scripts
which do not have intermediate access to the libvxdb C API, but still want to
authenticate a user. <i>tryauth</i> accepts the username and password on
standard input and the exit status contains the result of the authentication.
<code>0</code> is returned on success, <code>127</code> if there was a problem
during option parsing, or any other value indicating the PAM error code. If the
<code>-v</code> option is specified, additional error messages&nbsp;-- if
any&nbsp;-- will be printed to the standard error output.</p>

<?php include_once("Base-footer.php"); ?>
