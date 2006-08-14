<?php include_once("Base-header.php"); ?>

<h1>Synopsis</h1>

<p class="code"><tt>vxcl_randpw <b>[</b>-01BCDJMZcr<b>]</b> <b>[</b>-l
ARG<b>]</b></tt></p>

<h1>Description</h1>

<p class="block"><i>randpw</i> is an interface to the Vitalnix password
generator. The password is output once in plain and once in encrypted form.
Various flags control the generation and encryption behavior.</p>

<h1>Options</h1>

<h2>Generation</h2>

<table border="1" class="bordered">
  <tr>
    <td class="t1"><tt>-0</tt></td>
    <td class="t1">Possibly have a digit in the password</td>
  </tr>
  <tr>
    <td class="t2"><tt>-1</tt></td>
    <td class="t2">Always have a digit in the password</td>
  </tr>
  <tr>
    <td class="t1"><tt>-C</tt></td>
    <td class="t1">Always have an uppercase character in the password</td>
  </tr>
  <tr>
    <td class="t2"><tt>-J</tt></td>
    <td class="t2">Use "GENPW_JP" generation method</td>
  </tr>
  <tr>
    <td class="t1"><tt>-Z</tt></td>
    <td class="t1">Use "GENPW_ZH" generation method</td>
  </tr>
  <tr>
    <td class="t2"><tt>-c</tt></td>
    <td class="t2">Possibly have an uppercase character in the password</td>
  </tr>
  <tr>
    <td class="t1"><tt>-l LENGTH</tt></td>
    <td class="t1">Password length</td>
  </tr>
  <tr>
    <td class="t2"><tt>-r</tt></td>
    <td class="t2">Use "random" generation method</td>
  </tr>
</table>

<h2>Encryption</h2>

<table border="1" class="bordered">
  <tr>
    <td class="t1"><tt>-B</tt></td>
    <td class="t1">Use Blowfish encryption</td>
  </tr>
  <tr>
    <td class="t2"><tt>-D</tt></td>
    <td class="t2">Use DES encryption (this might not be available on all
      platforms)</td>
  </tr>
  <tr>
    <td class="t1"><tt>-M</tt></td>
    <td class="t1">Use MD5 encryption (this might not be available on all
      platforms)</td>
  </tr>
</table>

<?php include_once("Base-footer.php"); ?>
