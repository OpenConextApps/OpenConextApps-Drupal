<?php
/**
 * @file
 * Definition of SurfConext.
 *
 * SURFconext Drupal module enables federated authentication on the SURFcontext
 * service based on the SAML standard.
 */

class SurfConext {
  /**
   * @var SimpleSAML_Auth_Simple
   *   The simpleSAMLAuthSimple object.
   */
  protected $simpleSAMLAuthSimple = NULL;

  /**
   * @var surfconext
   *   The SURFconext instance.
   */
  protected static $surfconext = NULL;

  // -------------------------------------------------------------- Constructor.
  /**
   * The constructor.
   */
  public function __construct() {
    // Will throw exceptions when failure.
    $this->init();
  }

  // ------------------------------------------------------- Functions (static).
  /**
   * Setup the surfconext object.
   *
   * Will initialize for usage and loads possible session.
   *
   * @return surfconext
   *   Returns object or FALSE.
   */
  public static function get() {
    // Enabled?
    if (!self::isEnabled()) {
      return FALSE;
    }

    // Create when needed.
    if (empty(self::$surfconext)) {
      try {
        self::$surfconext = new SurfConext();
      }
      catch (Exception $e) {
        // Ignore.
      }
    }

    // Return the object.
    return self::$surfconext;
  }

  /**
   * Initialize the simplesaml library by activating its autoloader.
   */
  public static function autoLoad() {
    // Get the simplesaml library location.
    $basedir = self::getSimpleSamlBasedir();

    // Check if file exists.
    if (file_exists($basedir . '/lib/_autoload.php')) {
      // Autoload class.
      require_once $basedir . '/lib/_autoload.php';
      return TRUE;
    }

    return FALSE;
  }

  /**
   * Return the simplesamlphp version.
   *
   * @return string|boolean
   *   Return string or FALSE when failure.
   */
  public static function getSAMLVersion() {
    // Autoloader?
    if (self::autoLoad()) {
      // Get instance.
      $simplesaml_auth = SimpleSAML_Configuration::getInstance();

      // Get version.
      if ($simplesaml_auth) {
        return $simplesaml_auth->getVersion();
      }
    }

    return FALSE;
  }

  /**
   * For debug message logging.
   *
   * @param string $message
   *   The message to store in the log. Keep $message translatable
   *   by not concatenating dynamic values into it! Variables in the
   *   message should be added by using placeholder strings alongside
   *   the variables argument to declare the value of the placeholders.
   *   See t() for documentation on how $message and $variables interact.
   * @param array $variables
   *   Array of variables to replace in the message on display or
   *   NULL if message is already translated or not possible to
   *   translate.
   */
  public static function debug($message, array $variables = array()) {
    watchdog('SURFconext', $message, $variables, WATCHDOG_DEBUG);
  }

  /**
   * For normal (watchdog) logging.
   *
   * @param string $message
   *   The message to store in the log. Keep $message translatable
   *   by not concatenating dynamic values into it! Variables in the
   *   message should be added by using placeholder strings alongside
   *   the variables argument to declare the value of the placeholders.
   *   See t() for documentation on how $message and $variables interact.
   * @param array $variables
   *   Array of variables to replace in the message on display or
   *   NULL if message is already translated or not possible to
   *   translate.
   */
  public static function log($message, array $variables = array(), $severity = WATCHDOG_NOTICE) {
    watchdog('SURFconext', $message, $variables, $severity);
  }

  /**
   * Return the URL of the metadata IDP provider.
   *
   * @return string
   *   URL of the metadata IDP provider.
   */
  public static function getSurfConextMetadataIdpUrl() {
    return variable_get('surfconext_metadata_idp_url', 'https://engine.surfconext.nl/authentication/idp/metadata');
  }

  /**
   * Return the location of the metadata IDP provider file.
   *
   * @return string
   *   The location of the processed file metadata IDP provider.
   */
  public static function getSurfConextMetadataIdpFilename() {
    return variable_get('surfconext_metadata_idp_filename', 'private://saml20-idp-remote.php');
  }

  /**
   * Return the maximum life time of the metadata IDP provider file.
   *
   * @return integer
   *   The number of minutes of maximum life time. Default is 48 hours.
   */
  public static function getSurfConextMetadataIdpFileMaxAge() {
    return variable_get('surfconext_metadata_idp_file_max_age', 48 * 60);
  }

  /**
   * Return the number of minutes of the interval between updates for metadata.
   *
   * @return integer
   *   The number of minutes of the interval length.
   */
  public static function getSurfConextMetadataRefreshInterval() {
    return variable_get('surfconext_metadata_refresh_interval', 720);
  }

  /**
   * Return the simpleSAML PHP library location.
   *
   * @return string
   *   Location of the simpleSAML PHP library.
   */
  public static function getSimpleSamlBasedir() {
    return variable_get('surfconext_simplesaml_basedir', '/var/simplesamlphp');
  }

  /**
   * Return the Auth soure type needed for simpleSAML.
   *
   * @return string
   *   The simpleSAML source.
   */
  public static function getAuthSource() {
    return variable_get('surfconext_authsource', 'default-sp');
  }

  /**
   * Return with the attribute name for the unique name.
   *
   * @return string
   *   The name of the unique name.
   */
  public static function getAttributeNameUniqueID() {
    return variable_get('surfconext_attribute_name_unique_id', 'urn:mace:dir:attribute-def:eduPersonPrincipalName');
  }

  /**
   * Return with the attribute name for the unique name.
   *
   * @return string
   *   The name of the unique name.
   */
  public static function getAttributeNameMail() {
    return variable_get('surfconext_attribute_name_mail', 'urn:mace:dir:attribute-def:eduPersonPrincipalName');
  }

  /**
   * Return is the user is allowed to use Drupal login.
   *
   * @return boolean
   *   TRUE when Drupal login is allowed.
   */
  public static function allowDrupalLogin() {
    return variable_get('surfconext_allowDrupalLogin', TRUE);
  }

  /**
   * Return is the users that are allowed to use Drupal login.
   *
   * @return boolean
   *   TRUE when Drupal login is allowed.
   */
  public static function allowedDrupalLoginUsers() {
    return variable_get('surfconext_allowedDrupalLoginUsers', TRUE);
  }

  /**
   * Returns if SURFconext is activated.
   *
   * In most cases its better to use get(), unless you don't need to use the
   * SURFconext. The get() function will return FALSE when not enabled (or
   * something is wrong).
   *
   * @return boolean
   *   Returns TRUE when enabled, FALSE when not.
   */
  public static function isEnabled() {
    return variable_get('surfconext_enabled') ? TRUE : FALSE;
  }

  /**
   * Returns if we allow changing the links listing on certain blocks.
   *
   * @return boolean
   *   Returns TRUE when enabled, FALSE when not.
   */
  public static function isAltLoginEnabled() {
    return variable_get('surfconext_altlogin_enabled') ? TRUE : FALSE;
  }

  /**
   * Initialize SURFconext by loading the XML from the IDP url.
   */
  public static function cronSurfConext() {
    // Check if we need to update.
    $last_update = variable_get('surfconext_metadata_ipd_last_update');

    // Check if we need to update.
    if (($last_update + self::getSurfConextMetadataRefreshInterval()) > REQUEST_TIME) {
      // No time yet.
      return;
    }

    // Refresh the metadata file.
    self::refreshIdPMetadataFile();
  }

  /**
   * Refresh the metadata file.
   *
   * @throws SurfConextException
   */
  public static function refreshIdPMetadataFile() {
    // Get the location of the metadata file.
    $idp_filename = self::getSurfConextMetadataIdpFilename();

    // Check if we filename is set and writable. The requirements hook will
    // tell problems if the filename is not working.
    if (empty($idp_filename) || !is_writable(dirname(drupal_realpath($idp_filename)))) {
      return;
    }

    // Already running?
    if (!lock_acquire('surfconext_metadata_idp_get')) {
      return;
    }

    // Autoload lib.
    if (!self::autoLoad()) {
      throw new SurfConextException('Unable to autoload simpleSAML library.');
    }

    // Get url of the metadata.
    $url = self::getSurfConextMetadataIdpUrl();

    // Make sure what we get is valid, so admin can't hack it to get some file
    // from the system.
    if (valid_url($url, TRUE)) {
      try {
        // Get the metadata.
        $xmldata = file_get_contents($url);
        if (empty($xmldata)) {
          throw new SurfConextException('Unable to retrieve idp XML from SURFconext, check settings');
        }

        SimpleSAML_Utilities::validateXMLDocument($xmldata, 'saml-meta');
        $entities = SimpleSAML_Metadata_SAMLParser::parseDescriptorsString($xmldata);

        // Get all metadata for the entities.
        foreach ($entities as &$entity) {
          $entity = array(
            'saml20-idp-remote' => $entity->getMetadata20IdP(),
          );
        }

        // Convert to array.
        $output = SimpleSAML_Utilities::transposeArray($entities);

        // Get the metadata.
        $entity_metadata = reset($output['saml20-idp-remote']);

        // Remove the entityDescriptor element because it is unused, and
        // only makes the output harder to read.
        unset($entity_metadata['entityDescriptor']);

        $text = '<?php
/**
 * SAML 2.0 remote IdP metadata for simpleSAMLphp.
 *
 * See: https://rnd.feide.no/content/idp-remote-metadata-reference
 */

$metadata[\'' . $url . '\'] = ' . var_export($entity_metadata, TRUE) . ";\n";

        // Save to file.
        if (file_put_contents($idp_filename, $text) == FALSE) {
          self::log('Unable to write to metadata IdP file');
        }
        else {
          // Store when we did the last update.
          variable_set('surfconext_metadata_ipd_last_update', REQUEST_TIME);
        }
      }
      catch (Exception $e) {
        // Log it, but ignore.
        self::log('Unable to retrieve or process the metadata from SURFconext idp; @message', array('@message' => $e->getMessage()));
      }
    }
    else {
      self::log('Invalid URL detected for SURFconext metadat idp URL');
    }

    // Release lock.
    lock_release('surfconext_metadata_idp_get');
  }

  /**
   * Is user authenticated in SURFconext.
   *
   * @return boolean
   *   Returns TRUE when authenticated, or FALSE when not.
   */
  public static function isAuthenticated() {
    // Get instance.
    $surfconext = self::get();

    // Check surfconext.
    return $surfconext && $surfconext->samlIsAuthenticated() ? TRUE : FALSE;
  }

  /**
   * Goto the login page of SURFconext.
   */
  public function login() {

    // Make sure we are enabled.
    if (!self::isEnabled()) {
      drupal_set_message(t('SURFconext login is not enabled.'));
      drupal_goto();
    }

    if (!$this->simpleSAMLAuthSimple) {
      self::log('A user attempted to login using this module but the environment of SURFconext is missing, please check settings.', array(), WATCHDOG_CRITICAL);
      drupal_set_message(t('There was an unexpected problem. This problem has been logged for the administrator.'));
      drupal_goto();
    }

    // Trigger the SURFconext login.
    if (!user_is_logged_in()) {
      // Require the user to be authenticated.
      $this->simpleSAMLAuthSimple->requireAuth();
    }

    // If you set 'destination' in GET, then we will be redirected back.
    drupal_goto();
  }

  /**
   * Logout the Drupal session.
   *
   * Code copied from user_logout() in user.pages.inc.
   */
  public static function userLogout() {
    global $user;

    watchdog('user', 'Session closed for %name.', array('%name' => $user->name));

    module_invoke_all('user_logout', $user);

    // Destroy the current session, and reset $user to the anonymous user.
    session_destroy();

    drupal_goto();
  }


  /**
   * Get the SAML attributes.
   *
   * @return array|boolean
   *   The attributes array or FALSE.
   */
  public function getAttributes() {
    try {
      return empty($this->simpleSAMLAuthSimple) ? FALSE : $this->simpleSAMLAuthSimple->getAttributes();
    }
    catch (Exception $e) {
      self::log('Caught exception when trying to get attributes from SAML.');
    }
  }

  /**
   * Return with the attribute name for the unique name.
   *
   * @return string|boolean
   *   The value
   */
  public function getAttribute($name) {
    $attributes = $this->getAttributes();

    if (!empty($attributes) && isset($attributes[$name])) {
      return is_array($attributes[$name]) ? reset($attributes[$name]) : $attributes[$name];
    }

    return FALSE;
  }

  /**
   * Gets the authname attribute from the SAML attributes.
   *
   * @return string|boolean
   *   The authname attribute or FALSE.
   */
  protected function getAttributeAuthName() {
    return $this->getAttribute(self::getAttributeNameUniqueID());
  }

  /**
   * Gets value we use for ACL realm.
   *
   * @return string|boolean
   *   The authname attribute or FALSE.
   */
  public function getAttibuteUsedForRealm() {
    return drupal_strtolower($this->getAttributeAuthName());
  }

  /**
   * Gets the mail attribute from the SAML attributes.
   *
   * @return string|boolean
   *   The authname attribute or FALSE.
   */
  protected function getAttributeMail() {
    return $this->getAttribute(self::getAttributeNameMail());
  }

  /**
   * Initialize the session for SimpleSAML.
   *
   * @throws SurfConextException().
   */
  public function init() {
    // Must be enabled.
    if (!self::isEnabled()) {
      throw new SurfConextException('SURFconext module is not enabled.');
    }

    // Autoload lib.
    if (!self::autoLoad()) {
      throw new SurfConextException('Unable to autoload simpleSAML library.');
    }

    // Create the simpleSAMLAuthSimple object.
    $this->simpleSAMLAuthSimple = new SimpleSAML_Auth_Simple(self::getAuthSource());

    // Init based on login status on Drupal.
    user_is_logged_in() ? $this->initDrupalLoggedin() : $this->initDrupalLoggedOff();
  }

  /**
   * Is user authenticated in SURFconext.
   *
   * @return boolean
   *   Returns TRUE when authenticated, or FALSE when not.
   */
  protected function samlIsAuthenticated() {
    return $this->simpleSAMLAuthSimple->isAuthenticated() ? TRUE : FALSE;
  }

  /**
   * Initialize while user is logged into Drupal.
   */
  protected function initDrupalLoggedin() {
    global $user;

    // If we are authenticated on SURFconext, then we accept.
    if ($this->samlIsAuthenticated()) {
      return;
    }

    // Not authenticated on SURFconext.
    // Super user is always allowed as Drupal user without SURFconext.
    if ($user->uid == 1) {
      return;
    }

    // All other logged in Drupal users require SURFconext login.
    // userLogout() will not return.
    self::userLogout();
  }

  /**
   * Initialize while user is not logged into Drupal.
   */
  protected function initDrupalLoggedOff() {

    // Ignore if we are not authenticated for SURFconext.
    if (!$this->samlIsAuthenticated()) {
      return;
    }

    // User is logged into SURFconext, but not in Drupal.
    $authname = $this->getAttributeAuthName();
    self::debug("Authname is '%authname', not logged into Drupal.", array('%authname' => $authname));

    // No authname, then log and leave.
    if (empty($authname)) {
      self::debug("Can't continue init, authname not found in SAML attributes. Make sure ", array('%authname' => $authname));
      return;
    }

    // Get unique identifier from saml attributes.
    $authname = $this->getAttributeAuthName();
    if (empty($authname)) {
      return;
    }

    // Retrieve user mapping and attempt to log the user in.
    $ext_user = user_external_load($authname);

    // Create user?
    if (!$ext_user) {
      // Register in log.
      self::debug("Registering user '%authname'", array('%authname' => $authname));

      // Create user.
      user_external_login_register($authname, 'surfconext');

      // Get created account.
      $account = user_external_load($authname);
      if (!empty($account) && $account->uid) {

        // Get mail address user.
        $mail = $this->getAttributeMail();

        if (!empty($mail)) {
          // Extend user account.
          $userinfo = array(
            'mail' => drupal_strtolower($mail),
          );
          user_save($account, $userinfo);
        }
      }
    }
    else {
      global $user;
      $user = $ext_user;
    }

    // Finalizing the login, calls hook_user op login.
    $edit = array();
    user_login_finalize($edit);
  }

  /**
   * Called when user logs out of Drupal.
   *
   * @param object $account
   *   The account which is logging off (unused).
   */
  public function drupalLogoff($account) {

    // Ignore if we are not authenticated for SURFconext.
    if (!$this->samlIsAuthenticated()) {
      return;
    }

    // Destroy the current session, and reset $user to the anonymous user.
    session_destroy();

    // Logout of SURFconext.
    $this->simpleSAMLAuthSimple->logout(url('', array('absolute' => TRUE)));
  }
}
