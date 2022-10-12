using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class WeaponBaseClass : MonoBehaviour
{
    public BoxCollider _BoxCollider;
    public int _Damage;

    public int _Durability;
    public int _DurabilityCost;
    private int _currDurability;
    public int CurrDurability { get { return _currDurability; } set { _currDurability = value; } }

    private bool _shouldDestroy;
    public bool ShouldDestroy { get { return _shouldDestroy; } set { _shouldDestroy = value; } }

    public int _ForceMultiplier;
    private float _totalForce;
    public float TotalForce { get { return _totalForce; } set { _totalForce = value; } }

    private bool _ShouldApplyForce;
    public bool ShouldApplyForce { get { return _ShouldApplyForce; } set { _ShouldApplyForce = value; } }

    private int _playerID = -1;
    public int playerID { get { return _playerID; } set { _playerID = value; } }

    private PlayerMovementBehaviour _playerMovementBehavior = null;
    public PlayerMovementBehaviour _PlayerMovementBehavior { get { return _playerMovementBehavior; } set { _playerMovementBehavior = value; } }

    private LayerMask _attackWeaponLayer;
    public LayerMask AttackWeaponLayer { get { return _attackWeaponLayer; } set { _attackWeaponLayer = value; } }

    private LayerMask _defaultWeaponLayer;
    public LayerMask DefaultWeaponLayer { get { return _defaultWeaponLayer; } set { _defaultWeaponLayer = value; } }

    private float _objectforce;
    public float Objectforce { get { return _objectforce; } }

    protected virtual void Awake()
    {
        _defaultWeaponLayer = gameObject.layer;
        _currDurability = _Durability;
        TotalForce = GetComponent<Rigidbody>().mass * _ForceMultiplier;
        _objectforce = TotalForce / 15;
    }

    public virtual void ResetVariables()
    {
    }

}
