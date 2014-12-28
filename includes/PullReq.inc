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
      'msg' => "Error.",
    );
  }

  /**
   * @param $name Project name.
   * @return Leader name.
   */
  private function getLeader($name) {
    $result = db_query('SELECT leader FROM {bot_pullreq} WHERE name = :name',
      array(':name' => $name)
    )->fetchField();
    return $result;
  }

  /**
   * @param $name Project name.
   * @return Leader name.
   */
  private function getMembers($name) {
    $result = db_query('SELECT members FROM {bot_pullreq} WHERE name = :name',
      array(':name' => $name)
    )->fetchField();
    return $result;
  }

  /**
   * @param $name Project name.
   * @return Array members and leader.
   */
  private function getPeople($name) {
    $result = db_query('SELECT leader, members FROM {bot_pullreq} WHERE name = :name',
      array(':name' => $name)
    )->fetchAssoc();
    return $result;
  }

  /**
   * Add new project
   * @param $data Array with nick, channel and message.
   */
  public function setProject($data) {
    // Check if this project already exist.
    $result = $this->getLeader($data["message"][1]);

    if (empty($result)) {
      $result = db_query('INSERT INTO {bot_pullreq} (name, leader, dcreated) VALUES (:name, :leader, :dcreated)',
        array(
          ':name' => $data["message"][1],
          ':leader' => $data["nick"],
          ':dcreated' => time(),
        )
      );
      $this->result['msg'] = "Project created, well done!";
    }
    else {
      $this->result['msg'] = "Sorry " . $result . " registered that project before.";
    }

    return $this->result;
  }

  /**
   * Add member to project.
   * @param $data Array with nick, channel and message.
   */
  public function setMember($data) {
    // Check if this project already exist and get the leader.
    $result = $this->getPeople($data["message"][1]);

    if (empty($result)) {
      $this->result['msg'] = "Project not found.";
    }
    elseif ($result["leader"] != $data["nick"]) {
      $this->result['msg'] = "You are not authorized, contact with " . $result["leader"] . ".";
    }
    else {
      $space = empty($result["members"]) ? "" : " ";
      $result["members"] .= $space . implode(" ", array_slice($data["message"], 2));
      $result = db_query('UPDATE {bot_pullreq} SET members = :members WHERE name = :name',
        array(
          ':name' => $data["message"][1],
          ':members' => $result["members"],
        )
      );
      $this->result['msg'] = "More people to play the Russian roulette!";
    }

    return $this->result;
  }

  /**
   * Delete member from project
   * @param $data Array with nick, channel and message.
   */
  public function delMember($data) {
    // Check if this project already exist and get the owner.
    $result = $this->getPeople($data["message"][1]);

    if (empty($result["leader"])) {
      $this->result['msg'] = "Project not found.";
    }
    elseif ($result["leader"] != $data["nick"]) {
      $this->result['msg'] = "You are not authorized, contact with " . $result["leader"] . ".";
    }
    else {
      for ($i = 2; $i < count($data["message"]); $i++) {
        $result["members"] = str_replace($data["message"][$i] . " ", "", $result["members"]);
      }
      $result = db_query('UPDATE {bot_pullreq} SET members = :members WHERE name = :name',
        array(
          ':name' => $data["message"][1],
          ':members' => $result["members"],
        )
      );
      $this->result['msg'] = "ok... we'll miss them.";
    }

    return $this->result;
  }

  /**
   * Delete project
   * @param $data Array with nick, channel and message.
   */
  public function delProject($data) {
    // Check if this project already exist.
    $result = $this->getLeader($data["message"][1]);

    if (empty($result)) {
      $this->result['msg'] = "Project not found.";
    }
    elseif ($result == $data["nick"]) {
      $result = db_query('DELETE FROM {bot_pullreq} WHERE name = :name AND leader = :leader',
        array(
          ':name' => $data["message"][1],
          ':leader' => $data["nick"],
        )
      );
      $this->result['msg'] = "Project deleted, did you have a nice deploy?";
    }
    else {
      $this->result['msg'] = "You are not authorized, contact with " . $result["leader"] . ".";
    }

    return $this->result;
  }

  /**
   * Get all project members
   * @param $data Array with nick, channel and message.
   */
  public function getAllMembers($data) {
    // Check if this project already exist.
    $result = $this->getMembers($data["message"][1]);

    if (empty($result)) {
      $this->result['msg'] = "Project not found.";
    }
    else {
      $this->result['msg'] = "The full member list is: " . $result;
    }

    return $this->result;
  }

  /**
   * Get a random project member
   * @param $data Array with nick, channel and message.
   */
  public function pullreq($data) {
    // Check if this project already exist.
    $result = $this->getMembers($data["message"][1]);

    if (empty($result)) {
      $this->result['msg'] = "Project not found.";
    }
    else {
      $members = explode(" ", $result);
      $max_members = (count($members) - 1);
      $this->result['msg'] = "Our winner is .... : " . $members[rand(0, $max_members)] . "!!";
    }
    return $this->result;
  }
}
