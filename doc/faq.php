<?php include_once("zheader.php"); ?>

<h1>Glossary <img src="d_arr.png" /></h1>

<div class="pleft">
<table border="1" class="bordered">
  <tr>
    <td class="t1">libaccdb / ACCDB</td>
    <td class="t1">Short for "Account Database". It is a library which
      abstracts the way how exactly a user is added to a certain user database.
      Shadow, for example, accesses <tt>/etc/passwd</tt> while LDAP is in a
      different location.</td>
  </tr>
  <tr>
    <td class="t2">Backend Module</td>
    <td class="t2">Kind of a "driver" which speaks to a UDB and abstracts all
      this through ACCDB.</td>
  </tr>
  <tr>
    <td class="t1">C-spark</td>
    <td class="t1">A CLI-based tool for the batch administration of users. Part
      of the Vitalnix User Management Suite. (Could stand for <b>C</b>onsole
      version of a <b>S</b>ystem <b>P</b>rogram for <b>a</b>dding and
      <b>r</b>emoving ...)</td>
  </tr>
  <tr>
    <td class="t2">CLI</td>
    <td class="t2">Command Line Interface. Term for text-based applications
      that run on a tty rather than a display.</td>
  </tr>
  <tr>
    <td class="t1">CUI</td>
    <td class="t1">Console User Interface. Term for text-based "graphical"
      applications that run on a tty.</td>
  </tr>
  <tr>
    <td class="t2">UDB</td>
    <td class="t2">Short for "User Database", which could be
      <tt>/etc/passwd</tt>, an LDAP, ActiveDirectory or even a specialized
      MySQL database.</td>
  </tr>
  <tr>
    <td class="t1">Vitalnix</td>
    <td class="t1">Vitalnix is the package containing <i>libaccdb</i>,
      <i>C-Spark</i> and the fluff around it (such as <tt>vgroupadd</tt>). 
      C-Spark is in fact something separately, but is <i>just packaged</i>
      together with Vitalnix.</td>
  </tr>
  <tr>
    <td class="t2">Vitalnix Supply Programs</td>
    <td class="t2">Some programs and shell scripts that might come useful when
      working with the Vitalnix Suite.</td>
  </tr>
  <tr>
    <td class="t1">Vitalnix System Programs</td>
    <td class="t1">Essential tools to work with ACCDB from the command line.
      (In case you do not want to write your own program to interface the ACCDB
      API.)</td>
  </tr>
</table>
</div>

<?php include_once("zfooter.php"); ?>
