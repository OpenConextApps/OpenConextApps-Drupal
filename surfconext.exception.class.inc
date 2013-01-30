<?php
/**
 * @file
 * Definition of SurfConextException.
 */

class SurfConextException extends Exception {

  const SEVERITY_HIGH = 80;
  const SEVERITY_NORMAL = 50;
  const SEVERITY_LOW = 20;

  /**
   * The constructor.
   *
   * @param string $msg
   *   The exception message.
   * @param integer $severity
   *   The severity of the message, as per RFC 3164. Possible values are
   *   WATCHDOG_ERROR, WATCHDOG_WARNING, etc.
   * @param boolean $log
   *   Specify TRUE to log exception in Drupals watchdog. Default is TRUE.
   */
  public function __construct($msg = '', $severity = self::SEVERITY_NORMAL, $log = TRUE) {
    $msg = empty($msg) ? 'SURFconext exception was thrown.' : $msg;

    if ($severity >= self::SEVERITY_NORMAL || mediamosa_debug::is_debug()) {
      $log_serverity = $severity >= self::SEVERITY_HIGH ? WATCHDOG_CRITICAL : WATCHDOG_NOTICE;

      $levels = watchdog_severity_levels();

      // Get backtrace when you can.
      $backtrace = $this->getTraceAsString();

      if ($log) {
        watchdog(
          'SURFConext exception',
          '@message (Exception thrown (severity: @severity) on line @line, in @file)<br />Partial backtrace: <pre>@backtrace</pre>',
          array(
            '@severity' => $levels[$log_serverity],
            '@message' => $msg,
            '@line' => $this->getLine(),
            '@file' => $this->getFile(),
            '@backtrace' => empty($backtrace) ? 'no backtrace available' : $backtrace,
          ),
          $log_serverity
        );
      }
    }

    parent::__construct($msg);
  }
}
