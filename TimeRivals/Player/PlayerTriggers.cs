using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class PlayerTriggers : MonoBehaviour
{
    [SerializeField] private HealthSystem _healthSystem;
    [SerializeField] private PlayerMovementBehaviour _playerMovementBehavior;
    [SerializeField] private PlayerController _playerController;

    private void OnTriggerEnter(Collider other)
    {
        if (other.gameObject.CompareTag("Pickup"))
        {
            if (!_playerMovementBehavior.NearbyWeaponScript)
            {
                _playerMovementBehavior.NearbyWeaponScript = other.gameObject.GetComponent<WeaponPickup>();

                if (_playerMovementBehavior.CurrAttack == AttackIndexes.NONE) //If player doesn't have a weapon equiped
                    _playerMovementBehavior._pickupIndicator.SetActive(true);
            }
            return;
        }

        if (other.CompareTag("Hazard"))
        {
            _healthSystem.Damage(_healthSystem.MaxHealth, -1, other.gameObject);
        }
    }

    private void OnTriggerExit(Collider other)
    {
        if (other.gameObject.CompareTag("Pickup"))
        {
            if (other.gameObject.GetComponent<WeaponPickup>().BeingLootedByPlayerID == _playerController.PlayerID)
            {
                other.gameObject.GetComponent<WeaponPickup>().BeingLootedByPlayerID = -1;
            }
            _playerMovementBehavior.NearbyWeaponScript = null;
            _playerMovementBehavior._pickupIndicator.SetActive(false);
        }
    }
}
