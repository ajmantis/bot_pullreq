<?php

/**
 * Class PullReq
 */
class PullReq {
  // Something protected or private goes here.
  private $result;

  /**
   * Construct default values.
   */
  function __construct() {
    $result = array(
      'op' => 0,
      'msg' => t('Undefined error.'),
    );
  }

  /**
   * Leader getter.
   */
  private function getLeader($name) {
    $result = db_query('SELECT leader FROM {bot_pullreq} WHERE name = :name',
      array(':name' => $name)
    )->fetchField();
    return $result;
  }

  /**
   * Members getter.
   */
  private function getMembers($name) {
    $result = db_query('SELECT members FROM {bot_pullreq} WHERE name = :name',
      array(':name' => $name)
    )->fetchField();
    return $result;
  }

  /**
   * Everyone getter.
   */
  private function getPeople($name) {
    $result = db_query('SELECT leader, members FROM {bot_pullreq} WHERE name = :name',
      array(':name' => $name)
    )->fetchAssoc();
    return $result;
  }

  /**
   * Full project array getter.
   */
  public function getAllProjects() {
    $result = db_query('SELECT * FROM {bot_pullreq}');
    return $result;
  }

  /**
   * Add new project.
   * @param $data Array with nick, channel and message.
   */
  public function setProject($data) {
    // Check if this project already exist.
    $result = $this->getLeader($data['message'][1]);

    if (empty($result)) {
      $result = db_query('INSERT INTO {bot_pullreq} (name, leader, dcreated) VALUES (:name, :leader, :dcreated)',
        array(
          ':name' => $data['message'][1],
          ':leader' => $data['nick'],
          ':dcreated' => time(),
        )
      );
      $this->result['msg'] = t('Project created, well done!');
    }
    else {
      $this->result['msg'] = t('Sorry') . ' ' . $result . ' ' . t('registered that project before.');
    }

    return $this->result;
  }

  /**
   * Add member to project.
   * @param $data Array with nick, channel and message.
   */
  public function setMember($data) {
    // Check if this project already exist and get the leader.
    $result = $this->getPeople($data['message'][1]);

    if (empty($result)) {
      $this->result['msg'] = t('Project not found.');
    }
    elseif ($result['leader'] != $data['nick']) {
      $this->result['msg'] = t('You are not authorized, contact with') . ' ' . $result['leader'] . '.';
    }
    else {
      $space = empty($result['members']) ? '' : ' ';
      $result['members'] .= $space . implode(' ', array_slice($data['message'], 2));
      $result = db_query('UPDATE {bot_pullreq} SET members = :members WHERE name = :name',
        array(
          ':name' => $data['message'][1],
          ':members' => $result['members'],
        )
      );
      $this->result['msg'] = t('More people to play the Russian roulette!');
    }

    return $this->result;
  }

  /**
   * Delete member from project.
   * @param $data Array with nick, channel and message.
   */
  public function delMember($data) {
    // Check if this project already exist and get the owner.
    $result = $this->getPeople($data['message'][1]);

    if (empty($result['leader'])) {
      $this->result['msg'] = t('Project not found.');
    }
    elseif ($result['leader'] != $data['nick']) {
      $this->result['msg'] = t('You are not authorized, contact with ') . $result['leader'] . '.';
    }
    else {
      for ($i = 2; $i < count($data['message']); $i++) {
        $result['members'] = str_replace($data['message'][$i] . ' ', '', $result['members']);
      }
      $result = db_query('UPDATE {bot_pullreq} SET members = :members WHERE name = :name',
        array(
          ':name' => $data['message'][1],
          ':members' => $result['members'],
        )
      );
      $this->result['msg'] = t("ok... we'll miss them.");
    }

    return $this->result;
  }

  /**
   * Delete project.
   * @param $data Array with nick, channel and message.
   */
  public function delProject($data) {
    // Check if this project already exist.
    $result = $this->getLeader($data['message'][1]);

    if (empty($result)) {
      $this->result['msg'] = t('Project not found.');
    }
    elseif ($result != $data['nick'] && $data['nick'] != 'ChanServ') {
      $this->result['msg'] = t('You are not authorized, contact with ') . $result['leader'] . '.';
    }
    else {
      $result = db_query('DELETE FROM {bot_pullreq} WHERE name = :name AND leader = :leader',
        array(
          ':name' => $data['message'][1],
          ':leader' => $result ,
        )
      );
      $this->result['msg'] = t('Project deleted, did you have a nice deploy?');
      $this->result['op'] = 1;
    }

    return $this->result;
  }

  /**
   * Get all members from project name.
   * @param $data Array with nick, channel and message.
   */
  public function getAllMembers($data) {
    // Check if this project already exist.
    $result = $this->getMembers($data['message'][1]);

    if (empty($result)) {
      $this->result['msg'] = t('Project not found.');
    }
    else {
      $this->result['msg'] = t('The full member list is: ') . $result;
    }

    return $this->result;
  }

  /**
   * Get a random member from name project.
   * @param $data Array with nick, channel and message.
   */
  public function pullreq($data) {
    // Check if this project already exist.
    $result = $this->getMembers($data['message'][1]);

    if (empty($result)) {
      $this->result['msg'] = t('Project not found.');
    }
    else {
      $members = explode(' ', $result);
      $shouter = array_search($data['nick'], $members);

      if ($shouter) {
        unset($members[$shouter]);
      }

      if (count($members) > 0) {
        $this->result['msg'] = t('Our winner is .... : ') . $members[array_rand($members)] . '!!';
      }
      else {
        $this->result['msg'] = t("Don't be lazy, do it yourself.");
      }
    }
    return $this->result;
  }
}
