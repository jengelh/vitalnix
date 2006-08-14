<?php include_once("Base-header.php"); ?>

<h1>Synopsis</h1>

<p class="code"><tt>vxcl_tryauth <b>[</b>-v<b>]</b></tt></p>

<h1>Description</h1>

<p class="block"><i>tryauth</i> is a helper utility for programs and/or scripts
which do not have intermediate access to the libvxpdb C API, but still want to
authenticate a user. <i>tryauth</i> accepts the username and password on
standard input and the exit status contains the result of the authentication.
<tt>0</tt> is returned on success, <tt>127</tt> if there was a problem during
option parsing, or any other value indicating the PAM error code. If the
<tt>-v</tt> option is specified, additional error messages&nbsp;-- if
any&nbsp;-- will be printed to <tt>stderr</tt>.</p>

<?php include_once("Base-footer.php"); ?>
