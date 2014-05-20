<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
  <head>
    <title>Korektor</title>
    <meta http-equiv="Content-Type" content="text/html; charset=utf-8" />

    <!-- JQuery -->
    <script src="//ajax.googleapis.com/ajax/libs/jquery/1.10.2/jquery.min.js" type="text/javascript"></script>

    <!-- Lindat styles -->
    <link rel="stylesheet" href="branding/public/css/lindat.css" type="text/css" />

    <!-- Korektor styles -->
    <link rel="stylesheet" href="korektor.css" type="text/css" />
  </head>

  <body id="lindat-services">
    <?php require('branding/header.htm')?>
    <div class="container">
      <!-- breadcrumb -->
      <ul class="breadcrumb">
        <li><a href="http://lindat.mff.cuni.cz/">LINDAT/CLARIN</a> <span class="divider">/</span></li>
        <li><a href="http://lindat.mff.cuni.cz/services/">Services</a> <span class="divider">/</span></li>
        <li class="active">Korektor</li>
      </ul>

      <!-- Service title -->
      <div style="text-align: center">
        <h1>Korektor</h1>
      </div>

      <!-- menu -->
      <div class="navbar center">
        <div class="navbar-inner" style="text-align: center">
          <div class="nav-collapse">
            <ul class="nav">
              <li <?php if ($main_page == 'info.php') echo ' class="active"'?>><a href="info.php">Basic Info</a></li>
              <li <?php if ($main_page == 'demo.php') echo ' class="active"'?>><a href="demo.php">Demo</a></li>
              <li <?php if ($main_page == 'api-reference.php') echo ' class="active"'?>><a href="api-reference.php">REST API Documentation</a></li>
            </ul>
          </div>
        </div>
      </div>

      <div class="main-content-container">
