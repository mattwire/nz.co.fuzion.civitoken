<?php

define('CIVITOKEN_FINANCIAL_TYPE_MEMBERSHIP', 2); // "Member Dues"

function latestcurrentmembership_civitoken_declare($token){
  $tokens = array(
    $token . '.type' => 'Latest Current Membership Type',
    $token . '.end_date' => 'Latest Current Membership End Date',
    $token . '.end_date_raw' => 'Latest Current Membership End Date (raw)',
    $token . '.start_date' => 'Latest Current Membership Start Date',
    $token . '.start_date_raw' => 'Latest Current Membership Start Date (raw)',
    $token . '.join_date' => 'Latest Current Membership Join Date',
    $token . '.join_date_raw' => 'Latest Current Membership Join Date (raw)',
    $token . '.fee' => 'Latest Current Membership Fee',
    $token . '.fee_total' => 'Latest Current Membership Fee Total',
    $token . '.status' => 'Latest Current Membership Status',
    $token . '.alltype' => 'Latest Membership Type',
    $token . '.allend_date' => 'Latest Membership End Date',
    $token . '.allend_date_raw' => 'Latest Membership End Date (raw)',
    $token . '.allstart_date' => 'Latest Membership Start Date',
    $token . '.allstart_date_raw' => 'Latest Membership Start Date (raw)',
    $token . '.alljoin_date' => 'Latest Membership Join Date',
    $token . '.alljoin_date_raw' => 'Latest Membership Join Date (raw)',
    $token . '.allfee' => 'Latest Membership Fee',
    $token . '.allfee_total' => 'Latest Current Membership Fee Total',
    $token . '.allstatus' => 'Latest Membership Status',
  );

  // Add lineitem tokens
  for ($i=0;$i<10;$i++) {
    $tokens[$token . '.fee_'.$i] = 'Latest Current Membership Fee (Line item '.$i.')';
    $tokens[$token . '.fee_'.$i.'_label'] = 'Latest Current Membership Fee Label (Line item '.$i.')';
    $tokens[$token . '.fee_'.$i.'_qty'] = 'Latest Current Membership Fee Quantity (Line item '.$i.')';
    $tokens[$token . '.fee_'.$i.'_unit_price'] = 'Latest Current Membership Fee Unit Price (Line item '.$i.')';
    $tokens[$token . '.fee_'.$i.'_tax'] = 'Latest Current Membership Fee Tax (Line item '.$i.')';
    $tokens[$token . '.allfee_'.$i] = 'Latest Membership Fee (Line item '.$i.')';
    $tokens[$token . '.allfee_'.$i.'_label'] = 'Latest Membership Fee Label (Line item '.$i.')';
    $tokens[$token . '.allfee_'.$i.'_qty'] = 'Latest Membership Fee Quantity (Line item '.$i.')';
    $tokens[$token . '.allfee_'.$i.'_unit_price'] = 'Latest Membership Fee Unit Price (Line item '.$i.')';
    $tokens[$token . '.allfee_'.$i.'_tax'] = 'Latest Membership Fee Tax (Line item '.$i.')';
  }

  return $tokens;
}


function latestcurrentmembership_civitoken_get($cid, &$value, $context){
  latestcurrentmembership_get_tokens($cid, $value, FALSE);
  latestcurrentmembership_get_tokens($cid, $value, TRUE);
}

function latestcurrentmembership_get_tokens($cid, &$value, $onlyActive) {
  $params = array(
    'version' => 3,
    'contact_id' => $cid,
    'sequential' => 1,
    'api.membership_type.getsingle' => 1,
    'options' => array('limit' => 1, 'sort' => 'end_date DESC'),
    //'return' => array('membership_end_date', 'end_date', 'start_date', 'status_id', 'minimum_fee', 'membership_name', 'id', 'membership_type_id'),
  );
  $prefix = 'all';
  if ($onlyActive) {
    $params['is_active'] = 1;
    $params['active_only'] = 1;
    $prefix = '';
  }

  // Only get memberships with financial type "Member Dues"
  $membershipTypes = civicrm_api3('MembershipType', 'get', array(
    'financial_type_id' => CIVITOKEN_FINANCIAL_TYPE_MEMBERSHIP,
  ));
  foreach ($membershipTypes['values'] as $typeId => $val) {
    $types[] = $val['name'];
  }
  $params['membership_type_id'] = array('IN' => $types);

  static $statuses;
  if (empty($statuses)) {
    $statuses = civicrm_api3('membership', 'getoptions', array('field' => 'status_id'));
    $statuses = $statuses['values'];
  }
  try {
    $membership = civicrm_api3('membership', 'getsingle', $params);
  }
  catch (Exception $e) {
    $tokens = array('type', 'end_date', 'start_date', 'join_date', 'fee', 'status');
    foreach ($tokens as $token) {
      $value['latestcurrentmembership.' . $prefix . $token] = NULL;
    }
    return $value;
  }

  // Get the contribution_id (we just want the latest membership payment)
  $paymentParams = array(
    'membership_id' => $membership['id'],
    'options' => array('limit' => 1, 'sort' => "id DESC"),
  );
  try {
    $membershipPayment = civicrm_api3('MembershipPayment', 'getsingle', $paymentParams);
  }
  catch (Exception $e) {
    Civi::log()->warning('nz.co.fuzion.civitoken - Could not get membershipPayment: '.$e->getMessage());
    return array();
  }

  try {
    // Get the contribution (for total amount)
    $contribution = civicrm_api3('Contribution', 'getsingle', array(
      'id' => $membershipPayment['contribution_id'],
    ));
  }
  catch (Exception $e) {
    Civi::log()->warning('nz.co.fuzion.civitoken - Could not get membershipPayment: '.$e->getMessage());
    return array();
  }

  if (!CRM_Utils_Array::value('is_error', $contribution)) {
    $value['latestcurrentmembership.' . $prefix . 'fee_total'] = $contribution['total_amount'];
  }

  // Get the lineitems
  $lineItems = civicrm_api3('LineItem', 'get', array(
    'contribution_id' => $membershipPayment['contribution_id'],
  ));
  if (!CRM_Utils_Array::value('is_error', $lineItems)) {
    $feeId = 0;
    foreach ($lineItems['values'] as $id => $values) {
      $value['latestcurrentmembership.' . $prefix . 'fee_' . $feeId] = $values['line_total'];
      $value['latestcurrentmembership.' . $prefix . 'fee_' . $feeId . '_label'] = $values['label'];
      $value['latestcurrentmembership.' . $prefix . 'fee_' . $feeId . '_qty'] = (float)$values['qty'];
      $value['latestcurrentmembership.' . $prefix . 'fee_' . $feeId . '_unit_price'] = $values['unit_price'];
      $value['latestcurrentmembership.' . $prefix . 'fee_' . $feeId . '_tax'] = $values['tax_amount'];
      $feeId++;
    }
  }

  if (!CRM_Utils_Array::value('is_error', $membership)) {
    $value['latestcurrentmembership.' . $prefix . 'type'] = $membership['api.membership_type.getsingle']['name'];
    $value['latestcurrentmembership.' . $prefix . 'end_date'] = !empty($membership['end_date']) ? CRM_Utils_Date::customFormat($membership['end_date']) : '';
    $value['latestcurrentmembership.' . $prefix . 'end_date_raw'] = !empty($membership['end_date']) ? CRM_Utils_Date::customFormat($membership['end_date'],"%Y%m%d") : '';
    $value['latestcurrentmembership.' . $prefix . 'start_date'] = CRM_Utils_Date::customFormat($membership['start_date']);
    $value['latestcurrentmembership.' . $prefix . 'start_date_raw'] = CRM_Utils_Date::customFormat($membership['start_date'],"%Y%m%d");
    $value['latestcurrentmembership.' . $prefix . 'join_date'] = CRM_Utils_Date::customFormat($membership['join_date']);
    $value['latestcurrentmembership.' . $prefix . 'join_date_raw'] = CRM_Utils_Date::customFormat($membership['join_date'],"%Y%m%d");
    $value['latestcurrentmembership.' . $prefix . 'fee'] = $membership['api.membership_type.getsingle']['minimum_fee'];
    $value['latestcurrentmembership.' . $prefix . 'status'] = $statuses[$membership['status_id']];
  }
  return $value;
}
