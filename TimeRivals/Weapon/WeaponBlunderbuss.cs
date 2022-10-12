using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class WeaponBlunderbuss : WeaponBaseClass
{
    [SerializeField] private float _selfKnockbackForceMultiplier;
    public float SelfKnockbackForceMultiplier { get { return _selfKnockbackForceMultiplier; } }

    [SerializeField] private GameObject _bullet;
    [SerializeField] private int _bulletSpeed;
    [SerializeField] private Transform _bulletSpawnPoint;
    public Transform _bulletSpawnPoint2;
    public Transform _bulletSpawnPoint3;
    public bool ArchFiring;
    protected override void Awake()
    {
        base.Awake();
        
    }

    public void Fire()
    {
        GameObject bullet = Instantiate(_bullet, _bulletSpawnPoint.position, _bulletSpawnPoint.rotation);
        bullet.GetComponent<Bullet>().PlayerID = playerID;
        bullet.GetComponent<Rigidbody>().AddForce(transform.forward * _bulletSpeed, ForceMode.Impulse);
        GetComponent<Animator>().SetTrigger("Fire");

        if (ArchFiring == true)
        {
            GameObject bullet2 = Instantiate(_bullet, _bulletSpawnPoint2.position, _bulletSpawnPoint2.rotation);
            bullet2.GetComponent<Bullet>().PlayerID = playerID;
            bullet2.GetComponent<Rigidbody>().AddForce(_bulletSpawnPoint2.transform.forward * _bulletSpeed, ForceMode.Impulse);

            GameObject bullet3 = Instantiate(_bullet, _bulletSpawnPoint3.position, _bulletSpawnPoint3.rotation);
            bullet3.GetComponent<Bullet>().PlayerID = playerID;
            bullet3.GetComponent<Rigidbody>().AddForce(_bulletSpawnPoint3.transform.forward * _bulletSpeed, ForceMode.Impulse);
        }

        CurrDurability -= _DurabilityCost;
        
        FMODUnity.RuntimeManager.PlayOneShot("event:/Master/Weapons/Gun/Gun Shot Long", transform.position);

        if (CurrDurability <= 0)
        {
            ShouldDestroy = true;
        }
    }

    public override void ResetVariables()
    {
    }

}
