<?php

function latestcpdmembership_civitoken_declare($token){
  $tokens = array(
    $token . '.type' => 'Latest Current CPD Type',
    $token . '.end_date' => 'Latest Current CPD End Date',
    $token . '.end_date_raw' => 'Latest Current CPD End Date (raw)',
    $token . '.start_date' => 'Latest Current CPD Start Date',
    $token . '.start_date_raw' => 'Latest Current CPD Start Date (raw)',
    $token . '.join_date' => 'Latest Current CPD Join Date',
    $token . '.join_date_raw' => 'Latest Current CPD Join Date (raw)',
    $token . '.fee' => 'Latest Current CPD Fee',
    $token . '.fee_total' => 'Latest Current CPD Fee Total',
    $token . '.status' => 'Latest Current CPD Status',
    $token . '.alltype' => 'Latest CPD Type',
    $token . '.allend_date' => 'Latest CPD End Date',
    $token . '.allend_date_raw' => 'Latest CPD End Date (raw)',
    $token . '.allstart_date' => 'Latest CPD Start Date',
    $token . '.allstart_date_raw' => 'Latest CPD Start Date (raw)',
    $token . '.alljoin_date' => 'Latest CPD Join Date',
    $token . '.alljoin_date_raw' => 'Latest CPD Join Date (raw)',
    $token . '.allfee' => 'Latest CPD Fee',
    $token . '.allfee_total' => 'Latest Current CPD Fee Total',
    $token . '.allstatus' => 'Latest CPD Status',
  );

  // Add lineitem tokens
  for ($i=0;$i<10;$i++) {
    $tokens[$token . '.fee_'.$i] = 'Latest Current CPD Fee (Line item '.$i.')';
    $tokens[$token . '.fee_'.$i.'_label'] = 'Latest Current CPD Fee Label (Line item '.$i.')';
    $tokens[$token . '.fee_'.$i.'_qty'] = 'Latest Current CPD Fee Quantity (Line item '.$i.')';
    $tokens[$token . '.fee_'.$i.'_unit_price'] = 'Latest Current CPD Fee Unit Price (Line item '.$i.')';
    $tokens[$token . '.fee_'.$i.'_tax'] = 'Latest Current CPD Fee Tax (Line item '.$i.')';
    $tokens[$token . '.allfee_'.$i] = 'Latest CPD Fee (Line item '.$i.')';
    $tokens[$token . '.allfee_'.$i.'_label'] = 'Latest CPD Fee Label (Line item '.$i.')';
    $tokens[$token . '.allfee_'.$i.'_qty'] = 'Latest CPD Fee Quantity (Line item '.$i.')';
    $tokens[$token . '.allfee_'.$i.'_unit_price'] = 'Latest CPD Fee Unit Price (Line item '.$i.')';
    $tokens[$token . '.allfee_'.$i.'_tax'] = 'Latest CPD Fee Tax (Line item '.$i.')';
  }

  return $tokens;
}


function latestcpdmembership_civitoken_get($cid, &$value, $context){
  latestcpdmembership_get_tokens($cid, $value, FALSE);
  latestcpdmembership_get_tokens($cid, $value, TRUE);
}

function latestcpdmembership_get_tokens($cid, &$value, $onlyActive) {
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

  // Only get memberships with type "CPD"
  $params['membership_type_id'] = "CPD";

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
      $value['latestcpdmembership.' . $prefix . $token] = NULL;
    }
    return $value;
  }

  // Get the contribution_id
  $membershipPayment = civicrm_api3('MembershipPayment', 'getsingle', array(
    'membership_id' => $membership['id'],
  ));

  // Get the contribution (for total amount)
  $contribution = civicrm_api3('Contribution', 'getsingle', array(
    'id' => $membershipPayment['contribution_id'],
  ));
  if (!CRM_Utils_Array::value('is_error', $contribution)) {
    $value['latestcpdmembership.' . $prefix . 'fee_total'] = $contribution['total_amount'];
  }

  // Get the lineitems
  $lineItems = civicrm_api3('LineItem', 'get', array(
    'contribution_id' => $membershipPayment['contribution_id'],
  ));
  if (!CRM_Utils_Array::value('is_error', $lineItems)) {
    $feeId = 0;
    foreach ($lineItems['values'] as $id => $values) {
      $value['latestcpdmembership.' . $prefix . 'fee_' . $feeId] = $values['line_total'];
      $value['latestcpdmembership.' . $prefix . 'fee_' . $feeId . '_label'] = $values['label'];
      $value['latestcpdmembership.' . $prefix . 'fee_' . $feeId . '_qty'] = $values['qty'];
      $value['latestcpdmembership.' . $prefix . 'fee_' . $feeId . '_unit_price'] = $values['unit_price'];
      $value['latestcpdmembership.' . $prefix . 'fee_' . $feeId . '_tax'] = $values['tax_amount'];
      $feeId++;
    }
  }

  if (!CRM_Utils_Array::value('is_error', $membership)) {
    $value['latestcpdmembership.' . $prefix . 'type'] = $membership['api.membership_type.getsingle']['name'];
    $value['latestcpdmembership.' . $prefix . 'end_date'] = !empty($membership['end_date']) ? CRM_Utils_Date::customFormat($membership['end_date']) : '';
    $value['latestcpdmembership.' . $prefix . 'end_date_raw'] = !empty($membership['end_date']) ? CRM_Utils_Date::customFormat($membership['end_date'],"%Y%m%d") : '';
    $value['latestcpdmembership.' . $prefix . 'start_date'] = CRM_Utils_Date::customFormat($membership['start_date']);
    $value['latestcpdmembership.' . $prefix . 'start_date_raw'] = CRM_Utils_Date::customFormat($membership['start_date'],"%Y%m%d");
    $value['latestcpdmembership.' . $prefix . 'join_date'] = CRM_Utils_Date::customFormat($membership['join_date']);
    $value['latestcpdmembership.' . $prefix . 'join_date_raw'] = CRM_Utils_Date::customFormat($membership['join_date'],"%Y%m%d");
    $value['latestcpdmembership.' . $prefix . 'fee'] = $membership['api.membership_type.getsingle']['minimum_fee'];
    $value['latestcpdmembership.' . $prefix . 'status'] = $statuses[$membership['status_id']];
  }
  return $value;
}
