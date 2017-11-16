<?php
function phones_civitoken_declare($token) {
  $locationTokens = phones_civitoken_declare_locations($token);
  $typeTokens = phones_civitoken_declare_types($token);
  return CRM_Utils_Array::crmArrayMerge($locationTokens, $typeTokens);
}

function phones_civitoken_declare_locations($token) {
  $locationTypes = civicrm_api3('phone', 'getoptions', array(
    'field' => 'location_type',
  ));

  if (!empty($locationTypes['count'])) {
    foreach ($locationTypes['values'] as $key => $value) {
      $cleanLocation = strtolower(preg_replace('/[^A-Za-z0-9]/', '', $value));
      $tokens["{$token}.{$cleanLocation}"] = 'Phone ' . "({$value})";
    }
  }
  // The following two are deprecated "legacy" tokens for compatibility.
  $tokens[$token . '.work_phone'] = '(Deprecated) Work Phone';
  $tokens[$token . '.mobiles'] = '(Deprecated) Mobile Phone';
  return $tokens;
}

function phones_civitoken_declare_types($token) {
  $phoneTypes = civicrm_api3('phone', 'getoptions', array(
    'field' => 'phone_type_id',
  ));

  if (!empty($phoneTypes['count'])) {
    foreach ($phoneTypes['values'] as $key => $value) {
      $cleanType = strtolower(preg_replace('/[^A-Za-z0-9]/', '', $value));
      $tokens["{$token}.{$cleanType}"] = 'Phone ' . "({$value})";
    }
  }
  return $tokens;
}

/**
 * Get Phone tokens
 * @param int $cid
 * @param array $value
 */
function phones_civitoken_get($cid, &$contactDetails) {
  $locationValues = phones_civitoken_get_location($cid, $contactDetails);
  $typeValues = phones_civitoken_get_type($cid, $contactDetails);
  $contactDetails = CRM_Utils_Array::crmArrayMerge($locationValues, $typeValues);
}

function phones_civitoken_get_location($cid, $contactDetails) {
  $locationTypes = civicrm_api3('phone', 'getoptions', array(
    'field' => 'location_type',
  ));

  if (!empty($locationTypes['count'])) {
    foreach ($locationTypes['values'] as $locationId => $locationName) {
      $cleanLocation = strtolower(preg_replace('/[^A-Za-z0-9]/', '', $locationName));
      $contactDetails["phones.{$cleanLocation}"] = phones_civitoken_getPhone_byLocation($cid, $locationName);
    }
  }
  // The next two are deprecated - required for "legacy" tokens
  $contactDetails['phones.work_phone'] = phones_civitoken_getPhone_byLocation($cid, 'Work');
  $contactDetails['phones.mobiles'] = phones_civitoken_getPhone_byLocation($cid, 'mobile');
  return $contactDetails;
}

function phones_civitoken_get_type($cid, $contactDetails) {
  $phoneTypes = civicrm_api3('phone', 'getoptions', array(
    'field' => 'phone_type_id',
  ));

  if (!empty($phoneTypes['count'])) {
    foreach ($phoneTypes['values'] as $typeId => $typeName) {
      $cleanType = strtolower(preg_replace('/[^A-Za-z0-9]/', '', $typeName));
      $contactDetails["phones.{$cleanType}"] = phones_civitoken_getPhone_byType($cid, $typeName);
    }
  }
  return $contactDetails;
}

/**
 * Function get additional specific phones
 *
 * @param $contactID
 * @param $contactDetails
 * @param $type
 *
 * @return string
 */
function phones_civitoken_getPhone_byLocation($contactID, $type) {
  $params = array(
    'contact_id' => $contactID,
    'options' => array('limit' => 1, 'sort' => "is_primary DESC"),
  );

  if ($type == 'mobile') {
    // This is deprecated - required for "legacy" tokens
    $params['phone_type_id'] = 2;
  }
  else {
    $params['location_type_id'] = CRM_Core_PseudoConstant::getKey('CRM_Core_DAO_Address', 'location_type_id', $type);
  }

  try {
    $phones = civicrm_api3('phone', 'getsingle', $params);
    if (!empty($phones['phone'])) {
      return $phones['phone'];
    }
  }
  catch (CiviCRM_API3_Exception $e) {
    // No phone found
    return '';
  }
}

function phones_civitoken_getPhone_byType($contactID, $type) {
  $params = array(
    'contact_id' => $contactID,
    'options' => array('limit' => 1, 'sort' => "is_primary DESC"),
  );

  $params['phone_type_id'] = CRM_Core_PseudoConstant::getKey('CRM_Core_DAO_Phone', 'phone_type_id', $type);

  try {
    $phones = civicrm_api3('phone', 'getsingle', $params);
    if (!empty($phones['phone'])) {
      return $phones['phone'];
    }
  }
  catch (CiviCRM_API3_Exception $e) {
    // No phone found
    return '';
  }
}
