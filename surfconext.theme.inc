<?php
/**
 * @file
 * Contains all theme related functions.
 */

/**
 * Generates the text for the log in block.
 */
function theme_surfconext_login_link() {
  global $user;

  if (!SurfConext::isEnabled()) {
    // Exit without executing.
    return;
  }

  // Check if valid local session exists..
  if (SurfConext::isAuthenticated()) {
    return '<p>' . t('Logged in as: %name', array('%name', $user->name)) . '<br />' . l(t('Log Out'), 'user/logout') . '</p>';
  }

  return '<p>' . l(t('SURFconext Log In'), 'user/login/surfconext') . '</p>';
}

/**
 * Show any authentication information on current user.
 *
 * Mainly intended for showing debug information.
 */
function theme_surfconext_info() {
  global $user;

  // Get the instance.
  $surfconext = SurfConext::get();
  if (!$surfconext) {
    return;
  }

  // Get the saml attributes.
  $attributes = $surfconext->getAttributes();

  // Init header.
  $header = array(t('Name'), t('Value'));

  $rows = array();
  foreach ($attributes as $key => $value) {
    $rows[] = array($key, implode(',', $value));
  }

  $form = array();
  $form['surfconext'] = array(
    '#title' => 'SURFConext authentication attributes',
    '#type' => 'fieldset',
    '#collapsed' => TRUE,
    '#collapsible' => TRUE,
    '#attributes' => array('class' => array('collapsible collapsed')),
    '#attached' => array('library' => array(array('system', 'drupal.collapse'))),
  );

  $form['surfconext']['contents'] = array(
    '#markup' => theme(
      'table',
      array(
        'header' => $header,
        'rows' => $rows,
        'empty' => t('No SURFconext attributes found.'),
      )
    ),
  );

  // Add info about Drupal user.
  // Init header.
  $header = array(t('Name'), t('Value'));
  $rows = array();
  if (user_is_logged_in()) {
    $rows = array(
      array('UID', $user->uid),
      array('User', $user->name),
      array('Mail', $user->mail),
    );
  }

  $form['surfconext']['drupal'] = array(
    '#markup' => theme(
      'table',
      array(
        'header' => $header,
        'rows' => $rows,
        'empty' => t('Drupal user is not logged in.'),
      )
    ),
  );

  return drupal_render($form);
}
