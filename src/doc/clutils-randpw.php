<?php include_once("Base-header.php"); ?>

<h1>Usage</h1>

<p class="code"><code>randpw <b>[</b>-01BCDJMZcr<b>]</b> <b>[</b>-l
<i>length</i><b>]</b></code></p>

<h1>Description</h1>

<p class="block"><i>randpw</i> is an interface to the Vitalnix password
generator. The password is output once in plain and once in encrypted form.
Various flags control the generation and encryption behavior.</p>

<h1>Options</h1>

<h2>Generation</h2>

<table border="1">
  <tr>
    <td class="t1n"><code>-0</code></td>
    <td class="t1">Possibly have a digit in the password</td>
  </tr>
  <tr>
    <td class="t2n"><code>-1</code></td>
    <td class="t2">Always have a digit in the password</td>
  </tr>
  <tr>
    <td class="t1n"><code>-C</code></td>
    <td class="t1">Always have an uppercase character in the password</td>
  </tr>
  <tr>
    <td class="t2n"><code>-J</code></td>
    <td class="t2">Use "GENPW_JP" generation method</td>
  </tr>
  <tr>
    <td class="t1n"><code>-Z</code></td>
    <td class="t1">Use "GENPW_ZH" generation method</td>
  </tr>
  <tr>
    <td class="t2n"><code>-c</code></td>
    <td class="t2">Possibly have an uppercase character in the password</td>
  </tr>
  <tr>
    <td class="t1n"><code>-l</b> <i>length</i></code></td>
    <td class="t1">Password length</td>
  </tr>
  <tr>
    <td class="t2n"><code>-r</code></td>
    <td class="t2">Use "random" generation method</td>
  </tr>
</table>

<h2>Encryption</h2>

<table border="1">
  <tr>
    <td class="t1n"><code>-B</code></td>
    <td class="t1">Use Blowfish encryption</td>
  </tr>
  <tr>
    <td class="t2n"><code>-D</code></td>
    <td class="t2">Use DES encryption (this might not be available on all
      platforms)</td>
  </tr>
  <tr>
    <td class="t1n"><code>-M</code></td>
    <td class="t1">Use MD5 encryption (this might not be available on all
      platforms)</td>
  </tr>
</table>

<?php include_once("Base-footer.php"); ?>
