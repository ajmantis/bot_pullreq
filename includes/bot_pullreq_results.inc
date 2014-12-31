<?php

/**
 * Browse all projects with their members.
 *
 * @see bot_pullreq_results_form_submit()
 * @see _user_permission()
 */
function bot_pullreq_results_form($form, &$form_state) {
  $options = array();
  $pullreq = new PullReq();
  $data = $pullreq->getAllProjects();

  $header = array(
    'name' => t('Name'),
    'leader' => t('Leader'),
    'dcreated' => t('Created'),
    'members' => t('Members'),
  );

  foreach ($data as $item) {
    $options[$item->name] = array(
      'name' => $item->name,
      'leader' => $item->leader,
      'dcreated' => format_date($item->dcreated, 'short'),
      'members' => (strlen($item->members) > 1) ? count(explode(' ', $item->members)) : 0,
    );
  }

  $form['table'] = array(
    '#type' => 'tableselect',
    '#header' => $header,
    '#options' => $options,
    '#multiple' => TRUE,
    '#element_validate' => array('_user_permission'),
    '#empty' => t('No projects found'),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Submit',
  );

  return $form;
}

/**
 * Submit function for bot_pullreq_results_form().
 *
 * Adds a submit handler/function to our form to send a successful
 * completion message to the screen.
 */
function bot_pullreq_results_form_submit($form, $form_state) {
  $pullreq = new PullReq();

  foreach ($form_state['values']['table'] as $name) {
    if (!empty($name)) {
      $result = $pullreq->delProject(array(
        'nick' => 'ChanServ',
        'message' => array(
          1 => $name
        ),
      ));
    }
  }

  if (empty($result['op'])) {
    if (!empty($result['msg'])) {
      drupal_set_message($result['msg'], 'error');
    }
    unset($form_state['values']['table']);
    // @todo: fix a rare bug here...
  }
  else {
    drupal_set_message($result['msg']);
  }
}

/**
 * Validates user permission.
 */
function _user_permission($element, &$form_state, $form) {
  global $user;
  if (empty($user->uid)) {
    form_error($element, t("You don't have any right here."));
  }
}
