<?php include_once("Base-header.php"); ?>

<h1>Name</h1>

<p>userdel&nbsp;-- Delete a user account</p>

<h1>Usage</h1>

<p class="code"><tt><b>vxuserdel</b> <b>[</b>-A <b>[</b><i>command</i><b>]]</b>
<b>[</b>-B <b>[</b><i>command</i><b>]]</b> <b>[</b>-M <i>backend</i><b>]</b>
<b>[</b>-r<b>]</b> <i>login</i></tt></p

<h1>Options</h1>

<table border="1" class="bordered">
  <tr>
    <td class="t1">-A <b>[</b><i>command</i><b>]</b></td>
    <td class="t1">Runs the specified command after the user was successfully
      added. If <tt>-A</tt> is given, but with a zero-length command string,
      the default command in the configuration file is not run.</td>
  </tr>
  <tr>
    <td class="t2">-B <b>[</b><i>command</i><b>]</b></td>
    <td class="t2">Runs the specified command before the user is added. It is
      run always, even if adding fails. If <tt>-B</tt> is given, but with a
      zero-length command string, the default command in the configuration file
      is not run.</td>
  </tr>
  <tr>
    <td class="t1">-M <i>backend</i></td>
    <td class="t1">Uses the specified backend rather than the default one
      listed in the ACCDB configuration file</td>
  </tr>
  <tr>
    <td class="t2">-r</td>
    <td class="t2">Removes the user's home directory including files located
      therein and its mail spool (assuming <tt>/var/spool/mail/</tt>).</td>
  </tr>
</table>

<h1>Description</h1>

<p class="block"><tt>userdel</tt> deletes the specified user. The <tt>-A</tt>,
<tt>-B</tt> and <tt>-M</tt> options are unique to Vitalnix. The configuration
file for <tt>userdel</tt> is <tt>/etc/vitalnix/userdel.conf</tt>.</p>

<?php include_once("Base-footer.php"); ?>
